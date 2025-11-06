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
#include <execution>

#define PARALLEL_EXECUTION

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
	const Matrix& cameraToWorld = camera.CalculateCameraToWorld();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	const float fov = tan(camera.fovAngle * TO_RADIANS / 2);

#if defined(PARALLEL_EXECUTION)
	//Parallel logic
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);
	for (uint32_t idx{}; idx < amountOfPixels; idx++) pixelIndices.emplace_back(idx);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin);
		});
#else
	//Synchronous logic (no threading)
	const uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	for (int pixelIndex{}; pixelIndex < amountOfPixels; ++pixelIndex)
	{
		RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(const Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix& cameraToWorld, const Vector3& cameraOrigin) const
{
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	Vector3 rayDirection{ 0,0,1.f };
	rayDirection.x = ((2 * ((px + 0.5f) / m_Width)) - 1) * aspectRatio * fov;
	rayDirection.y = (1 - 2 * ((py + 0.5f) / m_Height)) * fov;

	rayDirection = cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	ColorRGB finalColor{};

	Ray viewRay{ cameraOrigin, rayDirection };

	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		for (const auto& light : lights)
		{
			Vector3 lightDirection{ LightUtils::GetDirectionToLight(light, closestHit.origin) };
			Ray lightRay{};

			lightRay.origin = closestHit.origin;
			lightRay.min = 0.1f;
			lightRay.max = lightDirection.Normalize();
			lightRay.direction = lightDirection;

			if (m_ShadowsEnabled)
			{
				if (pScene->DoesHit(lightRay))
				{
					continue;
				}
			}


			const ColorRGB& radiance{ LightUtils::GetRadiance(light, closestHit.origin) };
			const float observedArea{ std::max(0.f, Vector3::Dot(closestHit.normal, lightRay.direction) / (lightRay.direction.Magnitude() * closestHit.normal.Magnitude())) };
			const ColorRGB& BRDFrgb{ materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, -rayDirection) };

			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				finalColor += ColorRGB{ observedArea, observedArea, observedArea };
				break;
			case LightingMode::Radiance:
				finalColor += radiance;
				break;
			case LightingMode::BRDF:
				finalColor += BRDFrgb;
				break;
			case LightingMode::Combined:
				finalColor += radiance * BRDFrgb * observedArea;
				break;
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	

}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::BRDF;
		break;

	}
}