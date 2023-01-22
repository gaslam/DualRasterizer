#pragma once
#include "pch.h"
#include "Texture.h"
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& assetFile);
	virtual ~Effect();

	Effect(const Effect& other) = delete;
	Effect(Effect&& other) = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect& operator=(Effect&& other) = delete;

	void SetDiffuseMap(dae::Texture* pDiffuseTexture, ID3D11Device* pDevice);
	void SetWorldViewMatrix(float* pData);
	ID3D11InputLayout* GetInputLayout();

	ID3DX11Effect* GetEffect() const { return m_pEffect; }
	void SetSamplerDesciption(dae::FilteringMethodState filteringState);
	ID3DX11EffectTechnique* GetTechnique();

private:
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectMatrixVariable* m_pMatWorldViewVariable{ nullptr };
	ID3D11SamplerState* m_pSamplerState{};
	ID3DX11EffectSamplerVariable* m_pEffectSamplerState{};
	dae::Texture* m_pDiffuseTexture{};

protected:
	ID3DX11Effect* m_pEffect{ nullptr };
	ID3D11InputLayout* m_pInputLayout{ nullptr };
	ID3DX11EffectTechnique* m_pTechnique{ nullptr };

	ID3D11Device* m_pDevice{};
	std::wstring m_AssetFile{};

	virtual void CreateInputLayout() = 0;
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	void Initialise();
};

