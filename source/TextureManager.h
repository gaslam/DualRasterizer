#pragma once
#include <unordered_map>
#include "Texture.h"

class TextureManager final
{
public:
	static dae::Texture* GetTexture(const std::string& filename);
	static void DeleteTextures();
private:
	static std::unordered_map<std::string, dae::Texture*>* m_Textures;
};
 
