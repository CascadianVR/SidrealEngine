#pragma once
#include <string>

namespace Texture
{
	typedef struct
	{
		unsigned int id;
		unsigned int index;
		std::string type;
		std::string path;
	} Texture;

	unsigned int CreateTexture(const char* path, unsigned int textureIndex);
	unsigned int CreateBindlessTexture(const char* path);
	void SetActiveTexture(unsigned int* shaderProgram, unsigned int* texture, unsigned int index);
	void SetActiveBindlessTexture(unsigned int* shaderProgram, unsigned int* texture);
}