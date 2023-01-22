#include "pch.h"
#include "Mesh3D.h"
#include "Effect.h"
#include "Camera.h"
#include "EffectPosTex.h"
#include "Matrix.h"

using namespace dae;

Mesh3D::Mesh3D(ID3D11Device* pDevice,Effect* pEffect, std::vector<dae::Vertex_In>& vertices, std::vector<uint32_t>& indices) :
	m_pDevice{ pDevice },
	m_pEffect{ pEffect },
	m_NumIndices{}
{

	//Create Vertex Buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(dae::Vertex_Out) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

	if (FAILED(result))
	{
		return;
	}

	//Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result))
	{
		return;
	}
}

Mesh3D::~Mesh3D()
{
	if (m_pDevice)
	{
		m_pDevice->Release();
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
	}

	if (m_pEffect)
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}
}

void Mesh3D::Render(ID3D11DeviceContext* pDeviceContext, Camera* pCamera) const
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(dae::Vertex_In);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};

	m_pEffect->GetTechnique()->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0,0);
	}
}

void Mesh3D::Update(Camera* pCamera)
{
	m_RotationTransform = dae::Matrix::CreateRotationZ(m_Pitch) * dae::Matrix::CreateRotationY(m_Yaw);
	m_WorldMatrix = m_ScaleTransform * m_RotationTransform * m_TranslationTransform;
	dae::Matrix worldViewProjectionMatrix{ m_WorldMatrix * pCamera->GetWorldViewProjectionMatrix()};
	m_pEffect->SetWorldViewMatrix(reinterpret_cast<float*>(&worldViewProjectionMatrix));

	if (typeid(*m_pEffect) == typeid(EffectPosTex))
	{
		auto effect = dynamic_cast<EffectPosTex*>(m_pEffect);
		Matrix invViewMatrix{ pCamera->GetInverseViewMatrix()};
		effect->SetInverseViewMatrix(reinterpret_cast<float*>(&invViewMatrix));

		Matrix worldMatrix{ m_WorldMatrix };
		effect->SetWorldMatrix(reinterpret_cast<float*>(&worldMatrix));
	}
}

void Mesh3D::SetEffectSamplerState(dae::FilteringMethodState state)
{
	m_pEffect->SetSamplerDesciption(state);
}
