#include "pch.h"
#include "Renderer.h"
#include "Mesh3D.h"
#include "Camera.h"
#include "Utils.h"
#include "EffectPosTransp.h"
#include "EffectPosTex.h"
#include "DirectXRenderer.h"
#include "TextureManager.h"

using namespace dae;

DirectXRenderer::DirectXRenderer(SDL_Window* pWindow, Camera* pCamera) :
	Renderer::Renderer(pWindow,pCamera)
{

	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if (result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
		return;
	}

	std::vector<dae::Vertex_In> vertices;
	std::vector<uint32_t> indices;
	Utils::ParseOBJ("Resources/FireFX.obj", vertices, indices);
	EffectPosTransp* fireShader{ new EffectPosTransp{m_pDevice,L"Resources/Fire.fx"} };
	fireShader->SetDiffuseMap(TextureManager::GetTexture("Resources/fireFX_diffuse.png"),m_pDevice);
	Mesh3D* mesh2 = new Mesh3D(m_pDevice, fireShader, vertices, indices);
	m_pMeshes3D.push_back(mesh2);
	m_EffectTypes.push_back(EffectTypes::fire);

	Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);


	EffectPosTex* vehicleShader{ new EffectPosTex{m_pDevice,L"Resources/PosCol3D.fx"} };
	vehicleShader->SetDiffuseMap(TextureManager::GetTexture("Resources/vehicle_diffuse.png"), m_pDevice);
	vehicleShader->SetSpecularMap(TextureManager::GetTexture("Resources/vehicle_specular.png"),m_pDevice);
	vehicleShader->SetNormalMap(TextureManager::GetTexture("Resources/vehicle_normal.png"), m_pDevice);
	vehicleShader->SetGlossinessMap(TextureManager::GetTexture("Resources/vehicle_gloss.png"), m_pDevice);
	Mesh3D* mesh = new Mesh3D{ m_pDevice,vehicleShader,vertices,indices };
	m_pMeshes3D.push_back(mesh);
	m_EffectTypes.push_back(EffectTypes::other);
}
DirectXRenderer::~DirectXRenderer()
{
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
	}

	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
	}

	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
	}

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	for (Mesh3D* mesh : m_pMeshes3D)
	{
		if (mesh)
		{
			delete mesh;
			mesh = nullptr;
		}
	}
}
void DirectXRenderer::Update(const Timer* pTimer)
{
	if (m_CanRotate)
	{
		float rotationInRad{ (45.f * TO_RADIANS) };
					m_totalElapsedRotationTime += pTimer->GetElapsed();
		for (Mesh3D* mesh : m_pMeshes3D)
		{
			mesh->RotateY(rotationInRad * m_totalElapsedRotationTime);
			mesh->Update(m_pCamera);
		}
	}
}

void DirectXRenderer::Render()
{
	if (!m_IsInitialized)
		return;


	ColorRGB clearColorLightBlue = ColorRGB{ 0.3882352941176471f,0.5882352941176471f,0.9294117647058824f };
	if (m_UniformColorToggled)
	{
		clearColorLightBlue.r = static_cast<float>(m_UniformClearColorRGBvalue) / 255.f;
		clearColorLightBlue.g = static_cast<float>(m_UniformClearColorRGBvalue) / 255.f;
		clearColorLightBlue.b = static_cast<float>(m_UniformClearColorRGBvalue) / 255.f;
	}
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColorLightBlue.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	for (size_t i{}; i < m_pMeshes3D.size(); ++i)
	{
		Mesh3D* mesh{ m_pMeshes3D[i] };
		if (m_EffectTypes[i] == EffectTypes::fire && !m_FireToggled)
		{
			continue;
		}
		mesh->Render(m_pDeviceContext, m_pCamera);
	}
	m_pSwapChain->Present(0, 0);
}

HRESULT DirectXRenderer::InitializeDirectX()
{
	//1. Create Device & DeviceContext
	//=====
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
		1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
	if (FAILED(result))
	{
		return result;
	}
	//2. Create DXGI Factory
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
	if (FAILED(result))
	{
		return result;
	}

	//2. Create Swap Chain Description
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//3. Create Swap Chain
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

	if (FAILED(result))
	{
		return result;
	}

	//4. Create Depth Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;


	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);

	if (FAILED(result))
	{
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
	{
		return result;
	}

	//4. Create RenderTarget (RT) & RenderTargetView (RTV)
	//=====

	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
	{
		return result;
	}

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
	{
		return result;
	}
	//5. Bind RTV & DSV to Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	pDxgiFactory->Release();

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;

	result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pBackCullingRasterizerState);
	if (FAILED(result))
	{
		return result;
	}

	rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.FrontCounterClockwise = true;
	result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pFrontCullingRasterizerState);
	if (FAILED(result))
	{
		return result;
	}


	rasterDesc.CullMode = D3D11_CULL_NONE;
	result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pNoCullingRasterizerState);
	if (FAILED(result))
	{
		return result;
	}

	m_pDeviceContext->RSSetState(m_pBackCullingRasterizerState);
	return result;
}

void DirectXRenderer::ChangeFilteringMethod()
{
	int currentMethodIdx{ static_cast<int>(m_FilteringMethodState) };
	++currentMethodIdx;
	if (currentMethodIdx >= TotalFilteringMethodStates)
	{
		currentMethodIdx = 0;
	}

	m_FilteringMethodState = FilteringMethodState(currentMethodIdx);

	std::cout << "**(HARDWARE)Sampler Filter = ";

	switch (m_FilteringMethodState)
	{
	case dae::FilteringMethodState::point:
		std::cout << "POINT\n";
		break;
	case dae::FilteringMethodState::linear:
		std::cout << "LINEAR\n";
		break;
	case dae::FilteringMethodState::anisotropic:
		std::cout << "ANISOTROPIC\n";
		break;
	default:
		break;
	}
	for (Mesh3D* mesh : m_pMeshes3D)
	{
		mesh->SetEffectSamplerState(m_FilteringMethodState);
	}
}
void DirectXRenderer::ChangeCullMode()
{
	int currentState{ static_cast<int>(m_CullState) };
	++currentState;

	if (currentState == m_TotalCulStates)
	{
		currentState = 0;
	}

	m_CullState = CullState(currentState);

	switch (m_CullState)
	{
	case dae::DirectXRenderer::CullState::front:
		m_pDeviceContext->RSSetState(m_pFrontCullingRasterizerState);
		break;
	case dae::DirectXRenderer::CullState::back:
		m_pDeviceContext->RSSetState(m_pBackCullingRasterizerState);
		break;
	case dae::DirectXRenderer::CullState::none:
		m_pDeviceContext->RSSetState(m_pNoCullingRasterizerState);
		break;
	default:
		m_pDeviceContext->RSSetState(m_pFrontCullingRasterizerState);
		break;
	}

	D3D11_RASTERIZER_DESC desc{};
	m_pNoCullingRasterizerState->GetDesc(&desc);
}
