#pragma once
#include "Renderer.h"

struct SDL_Window;
struct SDL_Surface;
class Camera;
namespace dae
{
	class DirectXRenderer :
		public Renderer
	{
	public:
		DirectXRenderer(SDL_Window* pWindow, Camera* pCamera);
		virtual ~DirectXRenderer() override;

		DirectXRenderer(const DirectXRenderer&) = delete;
		DirectXRenderer(DirectXRenderer&&) noexcept = delete;
		DirectXRenderer& operator=(const DirectXRenderer&) = delete;
		DirectXRenderer& operator=(DirectXRenderer&&) noexcept = delete;

		virtual void Update(const Timer* pTimer) override;
		virtual void Render() override;

		void ChangeFilteringMethod();
		virtual void ChangeCullMode() override;

		void ToggleFire(bool canToggle) { m_FireToggled = canToggle; }

	private:
		bool m_IsInitialized{ false };
		bool m_FireToggled{ true };
		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		IDXGISwapChain* m_pSwapChain{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };
		std::vector<Mesh3D*> m_pMeshes3D{ };

		FilteringMethodState m_FilteringMethodState{};
		ID3D11SamplerState* m_pState{};
		ID3D11RasterizerState* m_pFrontCullingRasterizerState;
		ID3D11RasterizerState* m_pBackCullingRasterizerState;
		ID3D11RasterizerState* m_pNoCullingRasterizerState;

		enum class EffectTypes {
			fire,
			other
		};

		std::vector<EffectTypes> m_EffectTypes{};

		//DIRECTX
		HRESULT InitializeDirectX();
		//...
	};
}

