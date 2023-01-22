#include "pch.h"
#include "EffectPosTex.h"


EffectPosTex::EffectPosTex(ID3D11Device* pDevice, const std::wstring& assetFile) :
	Effect::Effect(pDevice, assetFile)
{
	m_pEffect = LoadEffect(pDevice,assetFile);
	Initialise();
	CreateInputLayout();

	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();

	if (!m_pMatWorldVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewVariable is not valid!!";
	}

	m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();

	if (!m_pMatViewInverseVariable->IsValid())
	{
		std::wcout << L"m_pMatViewInverseVariable is not valid!!";
	}
}

EffectPosTex::~EffectPosTex()
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}
	if (m_pGlossinesMapVariable)
	{
		m_pGlossinesMapVariable->Release();
	}
	Effect::~Effect();
}

void EffectPosTex::SetInverseViewMatrix(float* pData)
{
	m_pMatViewInverseVariable->SetMatrix(pData);
}

void EffectPosTex::SetWorldMatrix(float* pData)
{
	m_pMatWorldVariable->SetMatrix(pData);
}

void EffectPosTex::SetNormalMap(dae::Texture* pTexture, ID3D11Device* pDevice)
{
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();

	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable is not valid!!";
		return;
	}

	pTexture->SetSRV(pDevice);
	m_pNormalMapVariable->SetResource(pTexture->GetSRV());
	m_pNormalTexture = pTexture;
}

void EffectPosTex::SetGlossinessMap(dae::Texture* pTexture, ID3D11Device* pDevice)
{
	m_pGlossinesMapVariable = m_pEffect->GetVariableByName("gGlossinesMap")->AsShaderResource();

	if (!m_pGlossinesMapVariable->IsValid())
	{
		std::wcout << L"m_pGlossinesMapVariable is not valid!!";
		return;
	}
	pTexture->SetSRV(pDevice);
	m_pGlossinesMapVariable->SetResource(pTexture->GetSRV());
	m_pGlossTexture = pTexture;
}

void EffectPosTex::SetSpecularMap(dae::Texture* pTexture, ID3D11Device* pDevice)
{
	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();

	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable is not valid!!";
		return;
	}
	pTexture->SetSRV(pDevice);
	m_pSpecularMapVariable->SetResource(pTexture->GetSRV());
	m_pSpecularTexture = pTexture;
}

void EffectPosTex::CreateInputLayout()
{
	//Create Vertex Layout
	static constexpr uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "COLOR";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//Create Input Layout
	D3DX11_PASS_DESC passDesc{};

	GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = m_pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);

	if (FAILED(result))
	{
		return;
	}
}
