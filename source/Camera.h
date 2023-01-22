#pragma once

using namespace dae;
class Camera
{
public:
	Camera(Vector3 origin, float fovAngle, float aspectRatio);

	void Initialize(const float _aspect, float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, const float _far = 100.f, const float _near = .1f);
	void Update(const Timer* pTimer);

	Matrix GetViewMatrix() { return m_ViewMatrix; }
	Matrix GetProjectionMatrix() { return m_ProjectionMatrix; }
	Matrix GetWorldViewProjectionMatrix() { return m_ViewMatrix * m_ProjectionMatrix; }
	Matrix GetInverseViewMatrix() { return m_InvViewMatrix; }
	Vector3 GetOrigin() const { return m_Origin; }
	float GetFarDist() const { return m_Far; }
	float GetNearDist() const { return m_Near; }
private:

	Vector3 m_Origin{};
	float m_FovAngle{ 90.f };
	float m_Fov{ tanf((m_FovAngle * TO_RADIANS) / 2.f) };

	Vector3 m_Forward{ Vector3::UnitZ };
	Vector3 m_Up{ Vector3::UnitY };
	Vector3 m_Right{ Vector3::UnitX };

	float m_TotalPitch{};
	float m_TotalYaw{};
	float m_Aspect{};
	float m_Far{};
	float m_Near{};
	bool m_CanCalculateProjectionMatrix{};

	Matrix m_InvViewMatrix{};
	Matrix m_ViewMatrix{};
	Matrix m_ProjectionMatrix{};

	void CalculateViewMatrix();
	void HandleMouseRotation(const uint32_t& mouseState, const int mouseX, const int mouseY, const float speed, const float deltaTime);
	void HandleKeyMovement(const uint8_t* pKeyboardState, const float speed, const float deltaTime);
	void CalculateProjectionMatrix();
};

