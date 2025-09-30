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

	float aspectRatio = float(m_Width) / m_Height;

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//TODO W1: 6
			
			rayDirection.x = ((2 * ((float(px) + 0.5) / m_Width)) - 1) * aspectRatio;
			rayDirection.y = 1 - (2 * ((float(py) + 0.5) / m_Height));

			ColorRGB finalColor{};

			Ray viewRay{ {0,0,0}, rayDirection };

			HitRecord closestHit{};
			Plane testPlane{ {0.f,-50.f,0.f}, {0.f,1.f,0.f}, 0 };
			GeometryUtils::HitTest_Plane(testPlane, viewRay, closestHit);

			if(closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
			}

			
			//TODO W1: 4
			/*
			rayDirection.x = ((2 * ((float(px) + 0.5) / m_Width)) - 1) * aspectRatio;
			rayDirection.y = 1 - (2 * ((float(py) + 0.5) / m_Height));

			ColorRGB finalColor{};

			Ray viewRay{ {0,0,0}, rayDirection };

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if(closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
			}
			*/
			
			//TODO W1: 3
			/*
			rayDirection.x = ((2 * ((float(px) + 0.5) / m_Width)) - 1) * aspectRatio;
			rayDirection.y = 1 - (2 * ((float(py) + 0.5) / m_Height));

			ColorRGB finalColor{};

			Ray viewRay{ {0,0,0}, rayDirection };

			HitRecord closestHit{};

			Sphere testSphere{ {0,0,100}, 50.f, 0 };

			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if (closestHit.didHit)
			{
				const float scaled_t = (closestHit.t - 50.f) / 40.f;
				finalColor = { scaled_t, scaled_t, scaled_t };
			}
			*/
			
			//TODO W1: 2
			/*
			rayDirection.x = ((2 * ((float(px) + 0.5) / m_Width)) - 1) * aspectRatio;
			rayDirection.y = 1 - (2 * ((float(py) + 0.5) / m_Height));

			Ray viewRay{ {0,0,0}, rayDirection };
			
			HitRecord closestHit{};
			
			Sphere testSphere{ {0,0,100}, 50.f, 0 };
			
			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if(closestHit.didHit)
			{
				finalColor = materials[testSphere.materialIndex]->Shade(closestHit);
			}
			else
			{
				finalColor = { 0,0,0 };
			}
			*/
			
			//TODO W1: 1
			/*
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
			*/
			
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
