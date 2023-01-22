#pragma once
#include "Mesh3D.h"
#include "Camera.h"

struct SDL_Window;
class Camera;
namespace dae
{
	class Renderer
	{
	public:
		Renderer(SDL_Window* pWindow, Camera* pCamera);
		virtual ~Renderer()
		{

		}

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		virtual void Update(const Timer* pTimer) = 0;
		virtual void Render() = 0;
		virtual void ChangeCullMode() = 0;
		void ToggleRotation(bool canRotate) { m_CanRotate = canRotate; }
		void ToggleUniformColor(bool canTurnOn) { m_UniformColorToggled = canTurnOn; }
		float m_totalElapsedRotationTime{};

	protected:
		SDL_Window* m_pWindow{};
		enum class CullState
		{
			front,
			back,
			none
		};

		CullState m_CullState{CullState::front};
		Camera* m_pCamera;

		//Black
		Uint8 m_UniformClearColorRGBvalue{ 25 };

		int m_Width{};
		int m_Height{};

		bool m_CanRotate{ false };
		bool m_UniformColorToggled{ false };

		const int m_TotalCulStates{ 3 };
	};
}
