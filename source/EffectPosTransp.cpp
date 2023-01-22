#include "pch.h"
#include "EffectPosTransp.h"

EffectPosTransp::EffectPosTransp(ID3D11Device* pDevice, const std::wstring& assetFile) :
    Effect(pDevice, assetFile)
{
    m_pEffect = LoadEffect(pDevice, assetFile);
    Initialise();
    CreateInputLayout();
}

EffectPosTransp::~EffectPosTransp()
{
    Effect::~Effect();
}

void EffectPosTransp::CreateInputLayout()
{
    //Create Vertex Layout
    static constexpr uint32_t numElements{ 2 };
    D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

    vertexDesc[0].SemanticName = "POSITION";
    vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDesc[0].AlignedByteOffset = 0;
    vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    vertexDesc[1].SemanticName = "TEXCOORD";
    vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


    //Create Input Layout
    D3DX11_PASS_DESC passDesc{};

    GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

    HRESULT result = m_pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);

    if (FAILED(result))
    {
        return;
    }
}