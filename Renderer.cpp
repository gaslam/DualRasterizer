#include "pch.h"
#include "Renderer.h"
#include "Camera.h"

dae::Renderer::Renderer(SDL_Window* pWindow, Camera* pCamera) :
	m_pWindow{pWindow},
	m_pCamera{pCamera}
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

}

