#pragma once
#include "Matrix.h"
#include "Texture.h"
#include "Camera.h"
class Effect;
class Shader;
using namespace dae;
class Mesh3D final
{
public:
	Mesh3D(ID3D11Device* pDevice, Effect* pEffect, std::vector<dae::Vertex_In>& vertices, std::vector<uint32_t>& indices);
	~Mesh3D();

	Mesh3D(const Mesh3D& other) = delete;
	Mesh3D(Mesh3D& other) noexcept = delete;
	Mesh3D& operator=(const Mesh3D& other) = delete;
	Mesh3D& operator=(Mesh3D&& other) noexcept = delete;

	void Render(ID3D11DeviceContext* pDeviceContext, Camera* pCamera) const;
	void Update(Camera* pCamera);

	void Translate(const dae::Vector3& translation)
	{
		m_TranslationTransform = dae::Matrix::CreateTranslation(translation);
	}

	void Scale(const dae::Vector3& scale)
	{
		m_ScaleTransform = dae::Matrix::CreateScale(scale);
	}

	void RotateY(float yaw)
	{
		m_Yaw = yaw;
	}

	void RotateX(float pitch)
	{
		m_Pitch = pitch;
	}

	void SetEffectSamplerState(dae::FilteringMethodState state);

private:
	ID3D11Device* m_pDevice{ nullptr };
	ID3D11Buffer* m_pVertexBuffer{ nullptr };
	ID3D11Buffer* m_pIndexBuffer{ nullptr };

	uint32_t m_NumIndices;

	Effect* m_pEffect{nullptr};

	dae::Matrix m_WorldMatrix{};
	dae::Matrix m_RotationTransform{};
	dae::Matrix m_TranslationTransform{};
	dae::Matrix m_ScaleTransform{};

	float m_Yaw{};
	float m_Pitch{};
};

