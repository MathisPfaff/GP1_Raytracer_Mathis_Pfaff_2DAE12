#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera final
	{
		Camera() = default;
		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{}

		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			return { right, up, forward, origin };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			const float rotateSpeed{ 0.1f };
			const float moveSpeed{ (pKeyboardState[SDL_SCANCODE_LSHIFT]) ? 10.f : 5.f };

			if (pKeyboardState[SDL_SCANCODE_W]) 
			{
				origin += moveSpeed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_A]) 
			{
				origin -= moveSpeed * deltaTime * right;
			}
			if (pKeyboardState[SDL_SCANCODE_S]) 
			{
				origin -= moveSpeed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_D]) 
			{
				origin += moveSpeed * deltaTime * right;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			
			if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)))
			{
				origin += moveSpeed * mouseY * deltaTime * Vector3::UnitY;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				origin -= moveSpeed * mouseY * deltaTime * forward;

				totalYaw += rotateSpeed * mouseX * deltaTime;

				Matrix rotationMatrix{ Matrix::CreateRotationY(totalYaw) * Matrix::CreateRotationX(totalPitch) };

				forward = rotationMatrix.TransformVector(Vector3::UnitZ).Normalized();
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				totalPitch -= rotateSpeed * mouseY * deltaTime;
				totalYaw += rotateSpeed * mouseX * deltaTime;

				Matrix rotationMatrix{ Matrix::CreateRotationY(totalYaw) * Matrix::CreateRotationX(totalPitch) };

				forward = rotationMatrix.TransformVector(Vector3::UnitZ).Normalized();
			}
		}
	};
}
