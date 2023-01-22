#include "pch.h"
#include "Texture.h"
namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{pSurface},
		m_pSurfacePixels {(uint32_t*)pSurface->pixels}
	{
	}

	Texture::~Texture()
	{
		if (m_pSRV)
		{
			m_pSRV->Release();
			m_pSRV = nullptr;
		}
		if (m_pSurface != nullptr)
		{
			delete m_pSurface;
			m_pSurface = nullptr;
			SDL_FreeSurface(m_pSurface);
		}
		if (m_pResource)
		{
			m_pResource->Release();
			m_pResource = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)
		auto image = IMG_Load(path.c_str());
		return new Texture{ new SDL_Surface{ image->flags, image->format, image->w, image->h, image->pitch,image->pixels,  image->userdata, image->locked,image->lock_data,image->clip_rect,image->map,image->refcount }};
	}

	void Texture::SetSRV(ID3D11Device* pDevice)
	{
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};

		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

		if (FAILED(hr))
		{
			std::cout << "Could not create Texture2D!!\n";
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};

		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);

		if (FAILED(hr))
		{
			std::cout << "Could not create ShaderResourceView!!\n";
			return;
		}
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		const Vector2 uvToWidthHeight{ m_pSurface->w * uv.x , m_pSurface->h * uv.y };
		const Uint32 pixelIndex{ static_cast<Uint32>(uvToWidthHeight.x) + (static_cast<Uint32>(m_pSurface->w) * static_cast<Uint32>(uvToWidthHeight.y)) };
		try
		{
			const Uint32 pixel{ m_pSurfacePixels[pixelIndex] };
			Uint8 colorR{};
			Uint8 colorG{};
			Uint8 colorB{};
			SDL_GetRGB(pixel, m_pSurface->format, &colorR, &colorG, &colorB);
			const float colorRGBtoOne{ 1.f / 255.f };
			ColorRGB color{ colorRGBtoOne * static_cast<float>(colorR),   colorRGBtoOne * static_cast<float>(colorG),   colorRGBtoOne * static_cast<float>(colorB) };
			return color;
		}
		catch (...)
		{
			return {};
		}
	}
}