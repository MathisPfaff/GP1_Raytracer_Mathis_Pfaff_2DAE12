//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	Vector3 rayDirection{0,0,1};

	float aspectRatio{ m_Width / float(m_Height) };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//NDC
			rayDirection.x = ((2 * ((float(px) + 0.5) / m_Width)) - 1);
			rayDirection.y = 1 - (2 * ((float(py) + 0.5) / m_Height));
			
			//NSS
			//rayDirection.x = float(px) / m_Width;
			//rayDirection.y = float(py) / m_Height;
			
			//SS
			//rayDirection = Vector3{ float(px), float(py), 1 };
			
			Ray hitray{ {0, 0, 0}, rayDirection};
			ColorRGB finalColor{ rayDirection.x, rayDirection.y, rayDirection.z };
			
			
			//float gradient = px / static_cast<float>(m_Width);
			//gradient += py / static_cast<float>(m_Height);
			//gradient /= 2.0f;
			//
			//ColorRGB finalColor{ gradient, gradient, gradient };
			
			
			//Update Color in Buffer
			finalColor.MaxToOne();
			
			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
