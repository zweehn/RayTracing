#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "glm/glm.hpp"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::DragFloat("m_spherePos.x", &m_spherePos.x, 0.1, -10, 10);
		ImGui::DragFloat("m_spherePos.y", &m_spherePos.y, 0.1, -10, 10);
		ImGui::DragFloat("m_spherePos.z", &m_spherePos.z, 0.1, -10, 10);
		ImGui::DragFloat("m_sphereRadius", &m_sphereRadius, 0.1, 0, 10);
		
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

				int intersects = IntersectsSphere(rayStart, m_cameraFacing, m_spherePos, m_sphereRadius);

				uint8_t a = 0xFF;
				uint8_t r = 0x00;
				uint8_t g = 0x00;
				uint8_t b = 0x00;

				if (intersects != -1) {
					r = 0xFF;
					g = 0xFF;
					b = 0xFF;
				}

				uint8_t* vp = (uint8_t*)m_ImageData + (index * sizeof(uint32_t));
				vp[0] = r;
				vp[1] = g;
				vp[2] = b;
				vp[3] = a;
			}
		}

		m_Image->SetData(m_ImageData);

		m_LastRenderTime = timer.ElapsedMillis();
	}

	int IntersectsSphere(glm::vec3 rayStart, glm::vec3 rayDirection, glm::vec3 sphereCenter, float radius) {
		float a = powf(rayDirection.x, 2) + powf(rayDirection.y, 2) + powf(rayDirection.z, 2);
		float b = 2 * (rayStart.x * rayDirection.x - rayDirection.x * sphereCenter.x +
			rayStart.y * rayDirection.y - rayDirection.y * sphereCenter.y +
			rayStart.z * rayDirection.z - rayDirection.z * sphereCenter.z);
		float c = powf(rayStart.x, 2) - 2 * rayStart.x * sphereCenter.x + powf(sphereCenter.x, 2) +
			powf(rayStart.y, 2) - 2 * rayStart.y * sphereCenter.y + powf(sphereCenter.y, 2) +
			powf(rayStart.z, 2) - 2 * rayStart.z * sphereCenter.z + powf(sphereCenter.z, 2) -
			powf(radius, 2);

		if (powf(b, 2) - 4 * a * c >= 0) {
			float tp = -b + sqrtf(powf(b, 2) - 4 * a * c >= 0) / 2 * a;
			float tn = -b - sqrtf(powf(b, 2) - 4 * a * c >= 0) / 2 * a;
			if (tp >= 0 || tn >= 0) {
				return 1;
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}
	}
private:
	std::shared_ptr<Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	glm::vec3 m_cameraCenter = glm::vec3(0, -5, 1);
	glm::vec3 m_cameraFacing = glm::vec3(0, 1, 0);
	glm::vec3 m_cameraSize;
	float m_cameraWidth = 16;

	glm::vec3 m_spherePos = glm::vec3(2, 0, 1);
	float m_sphereRadius = 2;



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