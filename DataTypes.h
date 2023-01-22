#pragma once
#include <vector>
namespace dae
{

	struct Vertex_In
	{
		Vector3 position;
		Vector2 uv;
		ColorRGB color{ colors::White };
		Vector3 normal;
		Vector3 tangent;
	};
	struct Vertex_Out
	{
		Vector3 position;
		Vector2 Uv;
		ColorRGB color;
		Vector3 normal;
		Vector3 tangent;
	};

	struct Vertex_Out_Rasterizer
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
		bool ignore{};
	};

	enum class PrimitiveTopology
	{
		TriangeList,
		TriangleStrip
	};

	struct Mesh
	{
		std::vector<Vertex_In> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};
		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		float m_Yaw{};
		float m_Pitch{};

		void Update()
		{
			rotationTransform = Matrix::CreateRotationZ(m_Pitch) * Matrix::CreateRotationY(m_Yaw);
			worldMatrix = scaleTransform * rotationTransform * translationTransform;
		}

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void RotateY(float yaw)
		{
			m_Yaw = yaw;
		}

		void RotateX(float pitch)
		{
			m_Pitch = pitch;
		}
	};

	const int TotalFilteringMethodStates{ 3 };
	enum class FilteringMethodState
	{
		point,
		linear,
		anisotropic
	};
}