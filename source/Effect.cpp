#include "pch.h"
#include "Effect.h"
#include "Texture.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile) :
	m_AssetFile{ assetFile },
	m_pDevice{ pDevice }
{
}

Effect::~Effect()
{
	if (m_pTechnique != nullptr)
	{
		m_pTechnique->Release();
		m_pTechnique = nullptr;
	}

	if (m_pInputLayout != nullptr)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}

	if (m_pDiffuseMapVariable != nullptr)
	{
		m_pDiffuseMapVariable->Release();
		m_pDiffuseMapVariable = nullptr;
	}

	if (m_pMatWorldViewVariable != nullptr)
	{
		m_pMatWorldViewVariable->Release();
		m_pMatWorldViewVariable = nullptr;
	}

	if(m_pSamplerState != nullptr)
	{
		m_pSamplerState->Release();
		m_pSamplerState = nullptr;
	}

	if (m_pEffectSamplerState != nullptr)
	{
		m_pEffectSamplerState->Release();
		m_pEffectSamplerState = nullptr;
	}

	if (m_pEffect != nullptr)
	{
		m_pEffect->Release();
		m_pEffect = nullptr;
	}
}


void Effect::SetDiffuseMap(dae::Texture* pTexture, ID3D11Device* pDevice)
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();

	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable is not valid!!";
		return;
	}
	pTexture->SetSRV(pDevice);
	m_pDiffuseMapVariable->SetResource(pTexture->GetSRV());
	m_pDiffuseTexture = pTexture;
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };

	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;

			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to create effect from file!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}

void Effect::Initialise()
{

	m_pMatWorldViewVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();

	if (!m_pMatWorldViewVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldVariable is not valid!!";
	}

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"m_pTechnique is not valid!!";
	}


	m_pEffectSamplerState = m_pEffect->GetVariableByName("gStateToSample")->AsSampler();
	if (!m_pEffectSamplerState->IsValid())
	{
		std::wcout << L"m_pEffectSamplerState is not valid!!";
	}
	else
	{
		m_pEffectSamplerState->GetSampler(0, &m_pSamplerState);
	}
}

void Effect::SetWorldViewMatrix(float* pData)
{
	m_pMatWorldViewVariable->SetMatrix(pData);
}

void Effect::SetSamplerDesciption(dae::FilteringMethodState filteringState)
{
	D3D11_SAMPLER_DESC desc;
	m_pSamplerState->GetDesc(&desc);
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

	switch (filteringState)
	{
	case dae::FilteringMethodState::point:
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case dae::FilteringMethodState::linear:
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case dae::FilteringMethodState::anisotropic:
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
	default:
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	}

	m_pDevice->CreateSamplerState(&desc, &m_pSamplerState);
	if (m_pSamplerState != nullptr)
	{
		m_pEffectSamplerState->SetSampler(0, m_pSamplerState);
	}
}

ID3DX11EffectTechnique* Effect::GetTechnique()
{
	return m_pTechnique;
}

ID3D11InputLayout* Effect::GetInputLayout()
{
	return m_pInputLayout;
}
