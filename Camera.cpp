#include "pch.h"
#include "Camera.h"
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"
#include "Timer.h"


Camera::Camera(Vector3 origin, float fovAngle, float aspectRatio)
{
	Initialize(aspectRatio, fovAngle, origin);
}

void Camera::Initialize(const float _aspect, float _fovAngle, Vector3 _origin , const float _far, const float _near)
{
	m_FovAngle = _fovAngle;
	m_Fov = tanf((_fovAngle * TO_RADIANS) / 2.f);

	m_Origin = _origin;
	m_CanCalculateProjectionMatrix = true;
	m_Aspect = _aspect;
	m_Far = _far;
	m_Near = _near;
}

void Camera::CalculateViewMatrix()
{
	//ONB => invViewMatrix
	//Inverse(ONB) => ViewMatrix
	m_ViewMatrix = Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);
	m_InvViewMatrix = m_ViewMatrix.Inverse();

	//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]

	//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh


}

void Camera::HandleMouseRotation(const uint32_t& mouseState, const int mouseX, const int mouseY, const float speed, const float deltaTime)
{

	if ((mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
	{
		const float dir{ mouseY * speed };
		m_Origin.y += dir;
		return;
	}

	if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		float distance{ speed * deltaTime };
		if (distance > 1.f)
		{
			return;
		}
		if (mouseX != 0)
		{
			m_TotalYaw += mouseX * speed;
		}
		if (mouseY != 0)
		{
			Vector3 dir{ m_Forward * speed * 3.f * deltaTime};
			m_Origin += mouseY > 0 ? dir : -dir;
		}
	}

	if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
	{
		if (mouseY != 0)
		{
			m_TotalPitch += mouseY * speed;
		}
		if (mouseX != 0)
		{
			m_TotalYaw += mouseX * speed;
		}
	}

	const Matrix mc{ Matrix::CreateRotation(Vector3(-m_TotalPitch,m_TotalYaw,0.f)) };

	m_Forward = mc.TransformVector(Vector3::UnitZ);
	m_Forward.Normalize();
}


void Camera::HandleKeyMovement(const uint8_t* pKeyboardState, const float speed, const float deltaTime)
{
	const float lengthSquared{ m_Forward.SqrMagnitude() };
	if (pKeyboardState[SDL_SCANCODE_D])
	{
		Vector3 right{ Vector3::Cross(m_Up, m_Forward) * (speed * deltaTime)};
		m_Origin += right;
	}

	if (pKeyboardState[SDL_SCANCODE_A])
	{
		Vector3 left{ Vector3::Cross(m_Forward, m_Up) * (speed * deltaTime) };
		m_Origin += left;
	}

	if (pKeyboardState[SDL_SCANCODE_W])
	{

		m_Origin += m_Forward * (speed * deltaTime);
	}

	if (pKeyboardState[SDL_SCANCODE_S])
	{
		m_Origin -= m_Forward * (speed * deltaTime);
	}
}

void Camera::CalculateProjectionMatrix()
{
	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_Fov, m_Aspect, m_Near, m_Far);
	//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
	//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
}

void Camera::Update(const Timer* pTimer)
{
	const float deltaTime = pTimer->GetElapsed();

	//Camera Update Logic
	//...
	const float maxRotSpeed{ 100.f };
	const float rotSpeed{ 3.f };
	const float moveSpeed{ 20.f };
	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	//Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	HandleMouseRotation(mouseState, mouseX, mouseY, rotSpeed / maxRotSpeed , deltaTime);

	HandleKeyMovement(pKeyboardState, moveSpeed, deltaTime);

	//Update Matrices
	CalculateViewMatrix();

	if (m_CanCalculateProjectionMatrix)
	{
		CalculateProjectionMatrix();//Try to optimize this - should only be called once or when fov/aspectRatio changes
		m_CanCalculateProjectionMatrix = false;
	}
}
