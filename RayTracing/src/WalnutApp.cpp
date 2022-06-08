#include <cmath>
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "glm/glm.hpp"

using namespace Walnut;

struct ARGB_COLOR {
	uint8_t B;  // blue  channel
	uint8_t G;  // green channel
	uint8_t R;  // red   channel
	uint8_t A;  // alpha channel to indicate RGB.control true (!= 0) / false (== 0)
};

union COLOR {
	ARGB_COLOR argb;
	uint32_t value;

	COLOR operator * (float multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A * multiplier;
		c.argb.R = this->argb.R* multiplier;
		c.argb.G = this->argb.G* multiplier;
		c.argb.B = this->argb.B* multiplier;
		return c;
	}

	COLOR operator / (float multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A / multiplier;
		c.argb.R = this->argb.R / multiplier;
		c.argb.G = this->argb.G / multiplier;
		c.argb.B = this->argb.B / multiplier;
		return c;
	}

	COLOR operator + (float multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A + multiplier;
		c.argb.R = this->argb.R + multiplier;
		c.argb.G = this->argb.G + multiplier;
		c.argb.B = this->argb.B + multiplier;
		return c;
	}

	COLOR operator - (float multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A - multiplier;
		c.argb.R = this->argb.R - multiplier;
		c.argb.G = this->argb.G - multiplier;
		c.argb.B = this->argb.B - multiplier;
		return c;
	}

	COLOR operator * (COLOR multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A* multiplier.argb.A;
		c.argb.R = this->argb.R* multiplier.argb.R;
		c.argb.G = this->argb.G* multiplier.argb.G;
		c.argb.B = this->argb.B* multiplier.argb.B;
		return c;
	}

	COLOR operator / (COLOR multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A / multiplier.argb.A;
		c.argb.R = this->argb.R / multiplier.argb.R;
		c.argb.G = this->argb.G / multiplier.argb.G;
		c.argb.B = this->argb.B / multiplier.argb.B;
		return c;
	}

	COLOR operator + (COLOR multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A + multiplier.argb.A;
		c.argb.R = this->argb.R + multiplier.argb.R;
		c.argb.G = this->argb.G + multiplier.argb.G;
		c.argb.B = this->argb.B + multiplier.argb.B;
		return c;
	}

	COLOR operator - (COLOR multiplier) {
		COLOR c{};
		c.argb.A = this->argb.A - multiplier.argb.A;
		c.argb.R = this->argb.R - multiplier.argb.R;
		c.argb.G = this->argb.G - multiplier.argb.G;
		c.argb.B = this->argb.B - multiplier.argb.B;
		return c;
	}

} color;

struct PointLamp {
	COLOR Color;
	float Intensity;
	glm::vec3 Positon;
};

struct Intersection {
	bool IsIntersecting;
	glm::vec3 Normal;
	glm::vec3 Position;
};

struct Material {
	COLOR color;
	float Metalness;
};

class IRenderObject
{
public:
	Material mat;
	virtual Intersection intersect(glm::vec3 rayStart, glm::vec3 rayDirection) = 0;
};

class Sphere : public IRenderObject {
private:

	glm::vec3 GetSphereNormalAt(glm::vec3 SphereCenter, glm::vec3 SamplePoint) {
		return glm::normalize(SamplePoint - SphereCenter);
	}

	float intersectInternal(glm::vec3 rayStart, glm::vec3 rayDirection) {
		float a = glm::dot(rayDirection, rayDirection);
		float b = 2 * glm::dot(rayDirection, (rayStart - this->SpherePos));
		float c = glm::dot(rayStart - this->SpherePos, rayStart - this->SpherePos) - powf(this->Radius, 2);

		if (powf(b, 2) - 4 * a * c >= 0) {
			float tp = -b + sqrtf(powf(b, 2) - 4 * a * c) / (2 * a);
			float tn = -b - sqrtf(powf(b, 2) - 4 * a * c) / (2 * a);

			if (tp >= 0 || tn >= 0) {
				return fmax(fmin(tp, tn), 0) / 2;
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}
	}
public:
	glm::vec3 SpherePos;
	float Radius;

	Sphere(glm::vec3 SpherePos, float Radius, Material mat) {
		this->SpherePos = SpherePos;
		this->Radius = Radius;
		this->mat = mat;
	}

	Intersection intersect(glm::vec3 rayStart, glm::vec3 rayDirection) {
		float t = intersectInternal(rayStart, rayDirection);
		Intersection i;
		if (t != -1) {
			i.IsIntersecting = true;
			i.Position = rayStart + rayDirection * t;
			i.Normal = GetSphereNormalAt(this->SpherePos, i.Position);
		}
		else {
			i.IsIntersecting = false;
		}
		return i;
	}

};

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::DragFloat("m_spherePos.x", &m_Sphere.SpherePos.x, 0.1, -10, 10);
		ImGui::DragFloat("m_spherePos.y", &m_Sphere.SpherePos.y, 0.1, -10, 10);
		ImGui::DragFloat("m_spherePos.z", &m_Sphere.SpherePos.z, 0.1, -10, 10);
		ImGui::DragFloat("m_sphereRadius", &m_Sphere.Radius, 0.1, 0, 10);

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		SetupImage();

		ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void SetupImage() {
		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight())
		{
			m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);
			delete[] m_ImageData;
			m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
			float aspectRatio = 0;
			if (m_ViewportHeight != 0) {
				aspectRatio = static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight);
			}
			m_cameraSize = glm::vec3(m_cameraWidth, 0, m_cameraWidth / aspectRatio);
		}
	}

	void Render()
	{
		Timer timer;
		for (uint32_t y = 0; y < m_ViewportHeight; y++)
		{
			for (uint32_t x = 0; x < m_ViewportWidth; x++)
			{
				uint32_t index = y * m_ViewportWidth + x;

				glm::vec3 uv = (glm::vec3(static_cast<float>(x) / m_ViewportWidth, 0, static_cast<float>(y) / m_ViewportHeight) - 0.5f) * 2.0f;

				glm::vec3 rayStart = m_cameraCenter + m_cameraSize * uv;


				Intersection intersection = m_Sphere.intersect(rayStart, m_cameraFacing);


				COLOR PixelColor;
				PixelColor.argb.A = 0xFF;
				PixelColor.argb.R = ((uv.z + 1) / 2) * 128;
				PixelColor.argb.G = ((uv.z + 1) / 2) * 128;
				PixelColor.argb.B = 256 - ((uv.z + 1) / 2) * 200;

				if (intersection.IsIntersecting) {

					glm::vec3 vectoLight = glm::normalize(m_lamp.Positon - intersection.Position);

					float intensity = fmax(0,glm::dot(intersection.Normal, vectoLight));

					COLOR lampColor = m_lamp.Color * intensity;

					PixelColor = m_Sphere.mat.color + lampColor;
				}

				COLOR* vp;
				vp = (COLOR*)&m_ImageData[index];
				vp->value = PixelColor.value;
			}
		}

		m_Image->SetData(m_ImageData);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	std::shared_ptr<Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	glm::vec3 m_cameraCenter = glm::vec3(0, -50, 1);
	glm::vec3 m_cameraFacing = glm::normalize(glm::vec3(0, 1, 0));
	PointLamp m_lamp = PointLamp{
		{255, 0, 0, 0},
		1,
		{10,0,2}
	};
	glm::vec3 m_cameraSize = glm::vec3(0, 0, 0);
	float m_cameraWidth = 16;

	Sphere m_Sphere = Sphere(glm::vec3(2, 0, 1), 2, Material{ COLOR{ARGB_COLOR{0,0,0,255}},0});

	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
		});
	return app;
}