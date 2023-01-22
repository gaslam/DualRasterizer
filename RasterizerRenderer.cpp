//External includes
#include "pch.h"
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "RasterizerRenderer.h"

#include <algorithm>
#include <iostream>

#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "TextureManager.h"
#include "Utils.h"
#include <thread>
#include <future>
#include "MathHelpers.h"

#define ASYNC

using namespace dae;

RasterizerRenderer::RasterizerRenderer(SDL_Window* pWindow, Camera* pCamera) :
	Renderer::Renderer(pWindow,pCamera)
{
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	m_pTexture = TextureManager::GetTexture("Resources/vehicle_diffuse.png");
	m_pNormalTexture = TextureManager::GetTexture("Resources/vehicle_normal.png");
	m_pGlossTexture = TextureManager::GetTexture("Resources/vehicle_gloss.png");
	m_pSpecularTexture = TextureManager::GetTexture("Resources/vehicle_specular.png");
	Mesh* mesh{ new Mesh{} };
	Utils::ParseOBJ("Resources/vehicle.obj", mesh->vertices, mesh->indices);

	mesh->primitiveTopology = PrimitiveTopology::TriangeList;
	m_MeshesWorld.push_back(mesh);
	for (Mesh* mesh : m_MeshesWorld)
	{
		if (mesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			OptimiseWithTriangleStrip(mesh->indices);
		}
	}
}

RasterizerRenderer::~RasterizerRenderer()
{
	delete[] m_pDepthBufferPixels;
}


void RasterizerRenderer::Update(const Timer* pTimer)
{
	if (!m_CanRotate)
	{
		return;
	}
	const float fullRotationRadIn5Sec{45.f * TO_RADIANS };
	m_totalElapsedRotationTime += pTimer->GetElapsed();
	for (Mesh* mesh : m_MeshesWorld)
	{
		if (m_CanRotate)
		{
			mesh->RotateY(fullRotationRadIn5Sec * m_totalElapsedRotationTime);
		}
		mesh->Update();
	}
}

void dae::RasterizerRenderer::ChangeState()
{
	std::cout << "**(SOFTWARE) DepthBuffer Visualization ";
	m_State = m_State == RenderState::Texture ? RenderState::DepthBuffer : RenderState::Texture;
	switch (m_State) {
	case RenderState::Texture:
		std::cout << "OFF\n";
		break;
	case RenderState::DepthBuffer:
		std::cout << "ON\n";
		break;
	}
}

void dae::RasterizerRenderer::ChangeCullMode()
{
	int currentState{ static_cast<int>(m_CullState)};
	++currentState;
	if (currentState == m_TotalCulStates)
	{
		currentState = 0;
	}
	m_CullState = CullState(currentState);
	std::cout << "**(SHARED) CullMode = ";
	switch (m_CullState) {
	case CullState::front:
		std::cout << "FRONT\n";
		break;
	case CullState::back:
		std::cout << "BACK\n";
		break;

	case CullState::none:
		std::cout << "NONE\n";
		break;
	}
}

void dae::RasterizerRenderer::ChangeLightning()
{;
	int lighningModeIndex{ static_cast<int>(m_LightningMode) };
	++lighningModeIndex;
	if (lighningModeIndex >= m_TotalLightningModes)
	{
		lighningModeIndex = 0;
	}

	m_LightningMode = static_cast<LightningMode>(lighningModeIndex);

	std::cout << "**(SOFTWARE) Shading Mode = ";

	switch (m_LightningMode) {
	case LightningMode::ObservedArea:
		std::cout << "OBSERVED AREA\n";
		break;
	case LightningMode::Diffuse:
		std::cout << "DIFFUSE\n";
		break;
	case LightningMode::Specular:
		std::cout << "SPECULAR\n";
		break;
	case LightningMode::Combined:
		std::cout << "COMBINED\n";
		break;
	}
}

void RasterizerRenderer::Render()
{
	RenderMeshes();
}

void RasterizerRenderer::VertexTransformationFunction(const std::vector<Mesh*>& meshes, std::vector<Vertex_Out_Rasterizer>& vertices_out) const
{
	const float width{ static_cast<float>(m_Width) };
	const float height{ static_cast<float>(m_Height) };
	Matrix worldViewProjectionMatrix{};
	auto cameraWorldView{ m_pCamera->GetWorldViewProjectionMatrix() };
	for (const Mesh* mesh : meshes)
	{
		worldViewProjectionMatrix = mesh->worldMatrix * cameraWorldView;
		for (const Vertex_In& vert : mesh->vertices)
		{

			auto result = worldViewProjectionMatrix.TransformPoint({ vert.position,1.f });
			result.x /= result.w;
			result.y /= result.w;
			result.z /= result.w;
			Vertex_Out_Rasterizer projected{ result };

			projected.uv = vert.uv;
			projected.tangent = mesh->worldMatrix.TransformVector(vert.tangent);
			projected.normal = mesh->worldMatrix.TransformVector(vert.normal).Normalized();
			projected.color = vert.color;
			projected.viewDirection = mesh->worldMatrix.TransformPoint(vert.position) - m_pCamera->GetOrigin();

			vertices_out.emplace_back(projected);
		}
	}
}

void dae::RasterizerRenderer::OptimiseWithTriangleStrip(std::vector<Uint32>& indices)
{

	for (int i{ 1 }; i < indices.size() - 2; i += 2)
	{
		const Uint32 indice2{ indices[i + 1] }, indice3{ indices[i + 2] };
		indices[i + 1] = indice3;
		indices[i + 2] = indice2;
	}
}

void dae::RasterizerRenderer::RenderTriangle(const Vertex_Out_Rasterizer& v0, const Vertex_Out_Rasterizer& v1, const Vertex_Out_Rasterizer& v2, bool success, const SDL_Rect& boundaries)
{
	const int maxX{ boundaries.x + boundaries.w };
	const int maxY{ boundaries.y + boundaries.h };
	const Vector2 v0Pos{ v0.position.GetXY() };
	const Vector2 v1Pos{ v1.position.GetXY() };
	const Vector2 v2Pos{ v2.position.GetXY() };
	const float cameraFar{ m_pCamera->GetFarDist() };
	const float cameraNear{ m_pCamera->GetNearDist() };

	for (int px{ boundaries.x }; px <= maxX; ++px)
	{
		for (int py{ boundaries.y }; py <= maxY; ++py)
		{
			Vector2 screenSpacePos{ static_cast<float>(px),static_cast<float>(py) };
			ColorRGB finalColor{ 1.f,1.f,1.f };
			const int currentPixel{ px + (py * m_Width) };

			if (m_BoundingBoxToggled)
			{
				finalColor.MaxToOne();

				m_pBackBufferPixels[currentPixel] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
				continue;
			}

			Vector2 pointToSide = screenSpacePos - v0Pos;
			Vector2 a = v1Pos - v0Pos;
			float weight2 = Vector2::Cross(a, pointToSide);

			pointToSide = screenSpacePos - v1Pos;
			Vector2 b = v2Pos - v1Pos;
			float weight0 = Vector2::Cross(b, pointToSide);

			pointToSide = screenSpacePos - v2Pos;
			Vector2 c = v0Pos - v2Pos;
			float weight1 = Vector2::Cross(c, pointToSide);

			bool pointInTriangle{ };

			switch (m_CullState)
			{
			case CullState::back:
				pointInTriangle = (weight0 <= 0) && (weight1 <= 0) && (weight2 <= 0);
				break;
			case CullState::front:
				pointInTriangle = (weight0 >= 0) && (weight1 >= 0) && (weight2 >= 0);
				break;
			case CullState::none:
				pointInTriangle = ((weight0 >= 0) && (weight1 >= 0) && (weight2 >= 0)) || ((weight0 <= 0) && (weight1 <= 0) && (weight2 <= 0));
				break;
			}

			const float totalArea{ weight0 + weight1 + weight2 };
			weight0 /= totalArea;
			weight1 /= totalArea;
			weight2 /= totalArea;

			if (!pointInTriangle)
			{
				continue;
			}

			float lerpZ{ 1.f / ((1.f / v0.position.z) * weight0 + (1.f / v1.position.z) * weight1 + (1.f / v2.position.z) * weight2) };
			float currentDepthValue{ m_pDepthBufferPixels[currentPixel] };
			float lerpW{ (1.f / ((1.f / v0.position.w) * weight0 + (1.f / v1.position.w) * weight1 + (1.f / v2.position.w) * weight2)) };
			if (lerpZ < 0 || lerpZ > 1)
			{
				continue;
			}

			const bool isDepthSmaller{ currentDepthValue < lerpZ };
			if (isDepthSmaller)
			{
				success = false;
				continue;
			}
			m_pDepthBufferPixels[currentPixel] = lerpZ;
			switch (m_State)
			{
			case RenderState::Texture:
			{
				const Vector2 uv{ ((v0.uv / (v0.position.w)) * weight0 + (v1.uv / v1.position.w) * weight1 + (v2.uv / v2.position.w) * weight2) * lerpW };
				const Vector3 normal{ (((v0.normal / (v0.position.w)) * weight0 + (v1.normal / v1.position.w) * weight1 + (v2.normal / v2.position.w) * weight2) * lerpW).Normalized() };
				const Vector3 tangent{ (((v0.tangent / (v0.position.w)) * weight0 + (v1.tangent / v1.position.w) * weight1 + (v2.tangent / v2.position.w) * weight2) * lerpW).Normalized() };
				const Vector3 viewDir{ (((v0.viewDirection / (v0.position.w)) * weight0 + (v1.viewDirection / v1.position.w) * weight1 + (v2.viewDirection / v2.position.w) * weight2) * lerpW).Normalized() };
				const Vector4 pos{ screenSpacePos.x,screenSpacePos.y, lerpZ, lerpW };
				Vertex_Out_Rasterizer pixelVertex{ pos, finalColor, uv, normal, tangent, viewDir };
				finalColor = PixelShading(pixelVertex);
				break;
			}
			case RenderState::DepthBuffer:
			{
				Remap(lerpZ, .995f, 1.f);
				finalColor = { lerpZ, lerpZ, lerpZ };
				break;
			}
			}

			finalColor.MaxToOne();

			m_pBackBufferPixels[currentPixel] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
			success = true;
		}
	}
}

void dae::RasterizerRenderer::RenderMeshes()
{
	//@START
//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);
	std::vector<Vertex_Out_Rasterizer> vertices_ndc{};
	VertexTransformationFunction(m_MeshesWorld, vertices_ndc);
	Uint8 colorToMap{ 100 };

	if (m_UniformColorToggled)
	{
		colorToMap = m_UniformClearColorRGBvalue;
	}

	SDL_FillRect(m_pBackBuffer, &m_pFrontBuffer->clip_rect, SDL_MapRGB(m_pFrontBuffer->format, colorToMap, colorToMap, colorToMap));
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	bool succes{ false };
	for (Mesh* mesh : m_MeshesWorld)
	{
		if (mesh->primitiveTopology == PrimitiveTopology::TriangeList)
		{
			for (size_t i{}; i < mesh->indices.size(); ++i)
			{
				Vertex_Out_Rasterizer v0{ vertices_ndc[mesh->indices[i]] };
				Vertex_Out_Rasterizer v1{ vertices_ndc[mesh->indices[++i]] };
				Vertex_Out_Rasterizer v2{ vertices_ndc[mesh->indices[++i]] };


				if (v0.position.x < -1.f || v0.position.x > 1.f || v0.position.y < -1.f || v0.position.y > 1.f)
				{
					continue;
				}

				if (v1.position.x < -1.f || v1.position.x > 1.f || v1.position.y < -1.f || v1.position.y > 1.f)
				{
					continue;
				}

				if (v2.position.x < -1.f || v2.position.x > 1.f || v2.position.y < -1.f || v2.position.y > 1.f)
				{
					continue;
				}

				v0.position.x = (v0.position.x + 1) / 2.f * static_cast<float>(m_Width);
				v0.position.y = (1 - v0.position.y) / 2.f * static_cast<float>(m_Height);

				v1.position.x = (v1.position.x + 1) / 2.f * static_cast<float>(m_Width);
				v1.position.y = (1 - v1.position.y) / 2.f * static_cast<float>(m_Height);

				v2.position.x = (v2.position.x + 1) / 2.f * static_cast<float>(m_Width);
				v2.position.y = (1 - v2.position.y) / 2.f * static_cast<float>(m_Height);

				const int minX{ dae::Clamp(int(std::min(v0.position.x, std::min(v1.position.x, v2.position.x))),0,m_Width)};
				const int maxX{ dae::Clamp(int(std::max(v0.position.x, std::max(v1.position.x, v2.position.x))),0,m_Width)};
				const int minY{ dae::Clamp(int(std::min(v0.position.y, std::min(v1.position.y, v2.position.y))),0,m_Height) };
				const int maxY{ dae::Clamp(int(std::max(v0.position.y, std::max(v1.position.y, v2.position.y))),0,m_Height) };
				RenderTriangle(v0, v1, v2, succes, { minX,minY,maxX - minX,maxY - minY });
			}
		}

		if (mesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			for (size_t i{}; i < mesh->indices.size() - 2; ++i)
			{
				Vertex_Out_Rasterizer v0{ vertices_ndc[mesh->indices[i]] };
				Vertex_Out_Rasterizer v1{ vertices_ndc[mesh->indices[i + 1]] };
				Vertex_Out_Rasterizer v2{ vertices_ndc[mesh->indices[i + 2]] };

				if (i % 2 == 0)
				{
					v1 = vertices_ndc[mesh->indices[i + 2]];
					v2 = vertices_ndc[mesh->indices[i + 1]];
				}

				if (v0.position.x < -1.f || v0.position.x > 1.f || v0.position.y < -1.f || v0.position.y > 1.f)
				{
					continue;
				}

				if (v1.position.x < -1.f || v1.position.x > 1.f || v1.position.y < -1.f || v1.position.y > 1.f)
				{
					continue;
				}

				if (v2.position.x < -1.f || v2.position.x > 1.f || v2.position.y < -1.f || v2.position.y > 1.f)
				{
					continue;
				}

				v0.position.x = (v0.position.x + 1) / 2.f * static_cast<float>(m_Width);
				v0.position.y = (1 - v0.position.y) / 2.f * static_cast<float>(m_Height);

				v1.position.x = (v1.position.x + 1) / 2.f * static_cast<float>(m_Width);
				v1.position.y = (1 - v1.position.y) / 2.f * static_cast<float>(m_Height);

				v2.position.x = (v2.position.x + 1) / 2.f * static_cast<float>(m_Width);
				v2.position.y = (1 - v2.position.y) / 2.f * static_cast<float>(m_Height);

				const int minX{ static_cast<int>(std::max(0.f,std::min(v0.position.x - 1, std::min(v1.position.x - 1, v2.position.x - 1)))) };
				const int width{ static_cast<int>(std::min(static_cast<float>(m_Width),std::max(v0.position.x + 1, std::max(v1.position.x + 1, v2.position.x + 1)))) - minX };
				const int minY{ static_cast<int>(std::max(0.f,std::min(v0.position.y - 1, std::min(v1.position.y - 1, v2.position.y - 1)))) };
				const int height{ static_cast<int>(std::min(static_cast<float>(m_Height),std::max(v0.position.y + 1, std::max(v1.position.y + 1, v2.position.y + 1)))) - minY };
				RenderTriangle(v0, v1, v2, succes, { minX,minY,width,height });
				if (succes)
				{
					break;
				}
			}
		}
	}

	//@END
//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

static ColorRGB Lambert(float kd, const ColorRGB& cd)
{

	return cd * kd / PI;
}

static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
{

	return cd * kd / PI;
}

static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
{
	const Vector3 reflect{ Vector3::Reflect(l,n) };

	const float cosReflect{ ks * powf(std::max(0.0f,Vector3::Dot(reflect, v)), exp) };
	return { cosReflect, cosReflect, cosReflect };
}

ColorRGB RasterizerRenderer::Diffuse(const Vector2& uv, const float observedArea)
{
	const ColorRGB lightColor{ m_pTexture->Sample(uv) };
	const float lightIntensity{ 7.f };
	const ColorRGB radiance{ lightColor * lightIntensity };

	const auto lambert{ Lambert(observedArea,radiance) };
	return lambert;
}

ColorRGB RasterizerRenderer::Specular(const Vertex_Out_Rasterizer& v, const Vector3& vectorNormal, const Vector3& lightDirection)
{
	ColorRGB sampledSpecularColor{ m_pSpecularTexture->Sample(v.uv) };
	ColorRGB phongExponent{ m_pGlossTexture->Sample(v.uv) };
	const float shininess{ 25.f };

	const auto phong{ Phong(1.f ,phongExponent.r * shininess  ,lightDirection,-v.viewDirection,vectorNormal) };
	return  phong * sampledSpecularColor;
}

ColorRGB RasterizerRenderer::PixelShading(const Vertex_Out_Rasterizer& v)
{
	Vector3 lightDirection{ .577f,-.577f,.577f };
	Vector3 vectorNormal{ v.normal };

	if (v.uv.x < 0 || v.uv.x > 1
		|| v.uv.y < 0 || v.uv.y > 1) {
		return{};
	}

	if (m_ShowNormalMap)
	{
		Vector3 binormal{ Vector3::Cross(v.normal,v.tangent) };
		Matrix tangentSpaceAxis{ v.tangent,binormal,v.normal,Vector3::Zero };
		ColorRGB sampledNormalColor{ m_pNormalTexture->Sample(v.uv) };
		sampledNormalColor = 2.f * sampledNormalColor - 1.f;
		Vector3 sampledNormal{ Vector3(sampledNormalColor.r, sampledNormalColor.g, sampledNormalColor.b) };
		vectorNormal = tangentSpaceAxis.TransformVector(sampledNormal).Normalized();
	}
	float observedArea{ Vector3::Dot(-lightDirection, vectorNormal) };
	if (observedArea < 0.f)
	{
		observedArea = 0;
	}


	switch (m_LightningMode)
	{
	case LightningMode::ObservedArea:
	{

		return { observedArea,observedArea,observedArea };
	}
	case LightningMode::Diffuse:
		return Diffuse(v.uv, observedArea);
	case LightningMode::Specular:
		return Specular(v, vectorNormal, lightDirection);
	case LightningMode::Combined:
	{
		constexpr ColorRGB ambient{ 0.025f,0.025f,0.025f };
		return Diffuse(v.uv, observedArea) + Specular(v, vectorNormal, -lightDirection) + ambient;
	}
	default:
		break;
	}
	return {  };
}

void dae::RasterizerRenderer::Remap(float& depth, const float min, const float max)
{
	if (depth > max || depth < min)
	{
		return;
	}

	const float maxMinusMin{ max - min };
	const float depthMinusMin{ depth - min };
	depth = depthMinusMin / maxMinusMin;
}

bool dae::RasterizerRenderer::CanRenderTriangle(const Vector3& v1, const Vector3& v2, const Vector3& viewDir)
{

	Vector3 normal{ Vector3::Cross(v1,v2).Normalized() };

	if (m_CullState == CullState::front)
	{
		if (Vector3::Dot(viewDir, normal) > 0.f)
		{
			return false;
		}
	}
	else if (m_CullState == CullState::back)
	{
		if (Vector3::Dot(viewDir, normal) > 0.f)
		{
			return false;
		}
	}
	else if (m_CullState == CullState::none)
	{
		return false;
	}
	return true;
}

bool RasterizerRenderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
