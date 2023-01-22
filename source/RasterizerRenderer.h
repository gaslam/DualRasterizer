#pragma once
#include "Renderer.h"

struct SDL_Window;
struct SDL_Surface;
class MeshRasterizer;
class Camera;
namespace dae
{
	class RasterizerRenderer :
		public Renderer
	{
	public:
		RasterizerRenderer(SDL_Window* pWindow,Camera* pCamera);
		~RasterizerRenderer();

		RasterizerRenderer(const RasterizerRenderer& other) = delete;
		RasterizerRenderer(RasterizerRenderer&& other) noexcept = delete;
		RasterizerRenderer& operator=(const RasterizerRenderer& other) = delete;
		RasterizerRenderer& operator=(RasterizerRenderer&& other) noexcept = delete;

		virtual void Update(const Timer* pTimer) override;
		virtual void Render() override;
		virtual void ChangeCullMode() override;
		void ChangeState();
		void ChangeLightning();
		void ToggleNormalMap(bool canTurnOn) { m_ShowNormalMap = canTurnOn; }

		bool SaveBufferToImage() const;

		void ToggleBoundingBox(bool canToggleOn) { m_BoundingBoxToggled = canToggleOn; }

		enum class RenderState
		{
			Texture,
			DepthBuffer
		};
	private:
		const int m_TotalLightningModes{ 4 };
		enum class LightningMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined
		};

		LightningMode m_LightningMode{ LightningMode::Combined };

		bool m_BoundingBoxToggled{ false };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		Texture* m_pTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pGlossTexture{ nullptr };

		float* m_pDepthBufferPixels;
		RenderState m_State{ RenderState::Texture };
		std::vector<Mesh*> m_MeshesWorld{};

		bool m_ShowNormalMap{ true };

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Mesh*>& mesh, std::vector<Vertex_Out_Rasterizer>& vertices_out) const; //W1 Version
		void OptimiseWithTriangleStrip(std::vector<Uint32>& indices);
		void RenderTriangle(const Vertex_Out_Rasterizer& v0, const Vertex_Out_Rasterizer& v1, const Vertex_Out_Rasterizer& v2, bool success, const SDL_Rect& boundaries);
		void RenderMeshes();
		ColorRGB PixelShading(const Vertex_Out_Rasterizer& v);
		ColorRGB Diffuse(const Vector2& uv, float observedArea);
		ColorRGB Specular(const Vertex_Out_Rasterizer& v, const Vector3& vectorNormal, const Vector3& lightDirection);
		void Remap(float& depth, const float min, const float max);
		bool CanRenderTriangle(const Vector3& v1, const Vector3& v2, const Vector3& viewDir);

	};
}

