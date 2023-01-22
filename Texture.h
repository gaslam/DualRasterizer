#pragma once
namespace dae
{
	class Texture final
	{
	public:
		Texture(SDL_Surface* pSurface);
		~Texture();

		static Texture* LoadFromFile(const std::string& path);

		//Gets the Shader Resource View
		void SetSRV(ID3D11Device* pDevice);
		ID3D11ShaderResourceView* GetSRV() const { return m_pSRV; }
		ColorRGB Sample(const Vector2& uv) const;
	private:
		ID3D11Texture2D* m_pResource{nullptr};
		ID3D11ShaderResourceView* m_pSRV{nullptr};
		SDL_Surface* m_pSurface{nullptr};
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}