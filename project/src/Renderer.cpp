//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include <iostream>

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
	Matrix const& cameraToWorld = camera.CalculateCameraToWorld();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float const aspectRatio = float(m_Width) / m_Height;

	float const fov = tan(camera.fovAngle * TO_RADIANS / 2);

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 rayDirection{ 0,0,1.f };
			rayDirection.x = ((2 * ((px + 0.5f) / m_Width)) - 1) * aspectRatio * fov;
			rayDirection.y = (1 - 2 * ((py + 0.5f) / m_Height)) * fov;

			rayDirection = cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();

			ColorRGB finalColor{};

			Ray viewRay{ camera.origin, rayDirection };

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				float lightIntensity{ 1.f };

				for (Light const& light : lights)
				{
					Ray shadowRay{};
					shadowRay.origin = closestHit.origin + closestHit.normal * 0.001f;
					shadowRay.direction = (light.origin - closestHit.origin).Normalized();
					shadowRay.max = (light.origin - closestHit.origin).Magnitude();

					if (pScene->DoesHit(shadowRay))
					{
						lightIntensity /= 2;
					}
				}

				finalColor = materials[closestHit.materialIndex]->Shade() * lightIntensity;
				
			}
			
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
