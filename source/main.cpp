#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "DirectXRenderer.h"
#include "RasterizerRenderer.h"
#include "Camera.h"
#include "TextureManager.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

enum class ProgramState {
	rasterizer,
	directX
};

void PrintKeyBindings(const int greenConsoleAttribute, const int yellowConsoleAttribute, const int purpleConsoleAttribute, HANDLE& hConsole)
{

	SetConsoleTextAttribute(hConsole, yellowConsoleAttribute);
	std::cout << "[Key Bindings - SHARED]\n";
	std::cout << "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
	std::cout << "   [F2]  Toggle Vehicle Rotation (ON/OFF)\n";
	std::cout << "   [F9]  Cycle CullMode (BACK/FRONT/NONE)\n";
	std::cout << "   [F10] Toggle Uniform ClearColor (ON/OFF)\n";
	std::cout << "   [F11] Toggle Print FPS (ON/OFF)\n\n";

	SetConsoleTextAttribute(hConsole, greenConsoleAttribute);

	std::cout << "[Key Bindings - HARDWARE]\n";
	std::cout << "   [F3] Toggle FireFX (ON/OFF)\n";
	std::cout << "   [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n\n";

	SetConsoleTextAttribute(hConsole, purpleConsoleAttribute);

	std::cout << "[Key Bindings - SOFTWARE]\n";
	std::cout << "   [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n";
	std::cout << "   [F6] Toggle NormalMap (ON/OFF)\n";
	std::cout << "   [F7] Toggle DepthBuffer Visualization (ON/OFF)\n";
	std::cout << "   [F8] Toggle BoundingBox Visualization (ON/OFF)\n\n\n";


}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - ***Gaspard Lammertyn/2DAE08***",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize camera
	const float fovAngle{ 45.f };
	auto pCamera = new Camera{ {0,0,-50.f},fovAngle, static_cast<float>(width) / static_cast<float>(height) };

	//Initialize "framework"
	const auto pTimer = new Timer();
	const int totalRenderers{ 2 };
	int selectedMode{ 1 };
	RasterizerRenderer* pRasterizer = new RasterizerRenderer{ pWindow,pCamera };
	DirectXRenderer* pDirectX = new DirectXRenderer(pWindow, pCamera);
	Renderer* renderers[totalRenderers]{ pRasterizer, pDirectX };

	const int greenConsoleAttribute{ 2 };
	const int purpleConsoleAttribute{ 5 };
	const int yellowConsoleAttribute{ 6 };
	const int greyColorAttribute{ 8 };
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	PrintKeyBindings(greenConsoleAttribute, yellowConsoleAttribute, purpleConsoleAttribute, hConsole);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool canShowFPS{ false };
	bool isRotating{ true };
	bool isFireOn{ true };
	bool canShowNormalMap{ true };
	bool canClearUniformColor{ false };
	bool canToggleOn{ false };
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
#pragma region Key Bindings - SHARED
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					++selectedMode;
					if (selectedMode >= 2)
					{
						selectedMode = 0;
					}
					SetConsoleTextAttribute(hConsole, yellowConsoleAttribute);
					std::cout << "**(SHARED) Rasterizer Mode = ";
					//changes to software
					if (selectedMode == 0)
					{
						std::cout << " SOFTWARE\n";
					}
					else
					{
						std::cout << " HARDWARE\n";
					}
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					isRotating = !isRotating;
					for (int i{}; i < totalRenderers; ++i)
					{
						renderers[i]->ToggleRotation(isRotating);
					}
					SetConsoleTextAttribute(hConsole, yellowConsoleAttribute);
					std::cout << "**(SHARED) Vehicle Rotation ";

					if (isRotating)
					{
						std::cout << "ON\n";
					}
					else
					{
						std::cout << "OFF\n";
					}
				}


				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					SetConsoleTextAttribute(hConsole, yellowConsoleAttribute);
					for (int i{}; i < totalRenderers; ++i)
					{
						renderers[i]->ChangeCullMode();
					}
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					canClearUniformColor = !canClearUniformColor;
					for (int i{}; i < totalRenderers; ++i)
					{
						renderers[i]->ToggleUniformColor(canClearUniformColor);
					}
					SetConsoleTextAttribute(hConsole, yellowConsoleAttribute);
					std::cout << "**(SHARED) Uniform ClearColor = ";
					if (canClearUniformColor)
					{
						std::cout << "ON\n";
					}
					else
					{
						std::cout << "OFF\n";
					}

				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					canShowFPS = !canShowFPS;
					SetConsoleTextAttribute(hConsole, greyColorAttribute);
					std::cout << "**(SHARED)Print FPS ";
					if (canShowFPS)
					{
						std::cout << "ON\n";
					}
					else
					{
						std::cout << "OFF\n";
					}
				}
#pragma endregion
#pragma region Key Bindings - HARDWARE
				// 1 is hardware index
				if (selectedMode == 1)
				{
					if (e.key.keysym.scancode == SDL_SCANCODE_F3 && selectedMode == 1)
					{
						isFireOn = !isFireOn;
						pDirectX->ToggleFire(isFireOn);
						SetConsoleTextAttribute(hConsole, greenConsoleAttribute);
						std::cout << "**(HARDWARE) FireFX ";
						if (isFireOn)
						{
							std::cout << "ON\n";
						}
						else
						{
							std::cout << "OFF\n";
						}
					}
					if (e.key.keysym.scancode == SDL_SCANCODE_F4 && selectedMode == 1)
					{
						SetConsoleTextAttribute(hConsole, greenConsoleAttribute);
						pDirectX->ChangeFilteringMethod();
					}
				}
#pragma endregion
#pragma region Key Bindings - SOFTWARE
				// 0 is software index
				if (selectedMode == 0)
				{
					if (e.key.keysym.scancode == SDL_SCANCODE_F5)
					{
						SetConsoleTextAttribute(hConsole, purpleConsoleAttribute);
						pRasterizer->ChangeLightning();
					}
					if (e.key.keysym.scancode == SDL_SCANCODE_F6)
					{
						canShowNormalMap = !canShowNormalMap;
						SetConsoleTextAttribute(hConsole, purpleConsoleAttribute);
						pRasterizer->ToggleNormalMap(canShowNormalMap);
						std::cout << "NormalMap ";
						if (canShowNormalMap)
						{
							std::cout << "ON\n";
						}
						else
						{
							std::cout << "OFF\n";
						}
					}

					if (e.key.keysym.scancode == SDL_SCANCODE_F7)
					{
						SetConsoleTextAttribute(hConsole, purpleConsoleAttribute);
						pRasterizer->ChangeState();
					}
					if (e.key.keysym.scancode == SDL_SCANCODE_F8)
					{
						canToggleOn = !canToggleOn;
						SetConsoleTextAttribute(hConsole, purpleConsoleAttribute);
						pRasterizer->ToggleBoundingBox(canToggleOn);
						std::cout << "NormalMap ";
						if (canShowNormalMap)
						{
							std::cout << "ON\n";
						}
						else
						{
							std::cout << "OFF\n";
						}
					}
				}
#pragma endregion
				break;
			default:;
			}
		}

		//--------- Update ---------
		for (int i{}; i < totalRenderers; ++i)
		{
			renderers[i]->Update(pTimer);
		}
		pCamera->Update(pTimer);

		//--------- Render ---------
		renderers[selectedMode]->Render();

		//--------- Timer ---------
		pTimer->Update();
		if (canShowFPS)
		{
			printTimer += pTimer->GetElapsed();
			if (printTimer >= 1.f)
			{
				SetConsoleTextAttribute(hConsole, greyColorAttribute);
				printTimer = 0.f;
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();
	TextureManager::DeleteTextures();

	//Shutdown "framework"
	delete pTimer;
	delete pRasterizer;
	delete pCamera;
	delete pDirectX;

	ShutDown(pWindow);
	return 0;
}