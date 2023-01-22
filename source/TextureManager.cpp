#include "pch.h"
#include "TextureManager.h"
#include "Texture.h"

std::unordered_map<std::string, dae::Texture*>* TextureManager::m_Textures{ new std::unordered_map<std::string, dae::Texture*>{} };

dae::Texture* TextureManager::GetTexture(const std::string& filename)
{
	if (m_Textures->find(filename) == m_Textures->end())
	{
		std::pair<std::string, dae::Texture*> texture{ std::pair<std::string, dae::Texture*>(filename,dae::Texture::LoadFromFile(filename)) };
		m_Textures->insert(texture);
	}
	return m_Textures->at(filename);
}

void TextureManager::DeleteTextures()
{
	for (const std::pair<std::string, dae::Texture*>& pair : *m_Textures)
	{
		if (pair.second != nullptr)
		{
			delete pair.second;
		}
	}
	delete m_Textures;
	m_Textures = nullptr;
}