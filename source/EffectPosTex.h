#pragma once
#include "Effect.h"
class EffectPosTex : public Effect
{
public:
	EffectPosTex(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~EffectPosTex() override;

	void SetInverseViewMatrix(float* pData);

	void SetWorldMatrix(float* pData);

	void SetNormalMap(dae::Texture* pTexture, ID3D11Device* pDevice);

	void SetGlossinessMap(dae::Texture* pTexture, ID3D11Device* pDevice);

	void SetSpecularMap(dae::Texture* pTexture, ID3D11Device* pDevice);

	EffectPosTex(const EffectPosTex& other) = delete;
	EffectPosTex(EffectPosTex&& other) = delete;
	EffectPosTex& operator=(const EffectPosTex& other) = delete;
	EffectPosTex& operator=(EffectPosTex&& other) = delete;

	virtual void CreateInputLayout() override;


private:
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinesMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{  };
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable{ };
	dae::Texture* m_pNormalTexture;
	dae::Texture* m_pGlossTexture;
	dae::Texture* m_pSpecularTexture;
};