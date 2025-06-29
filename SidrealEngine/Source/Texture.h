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

	unsigned int LoadTexture2D(const char* path);
	unsigned int CreateTexture2D(const unsigned char* data, size_t dataSize);
	unsigned int LoadTextureHDR(const char* path);
	unsigned int CreateBindlessTexture(const char* path);
	void SetActiveAndBindTexture(unsigned int texture, unsigned int index);
	void SetActiveAndBindTextureArray(unsigned int texture, unsigned int index);
	void SetActiveBindlessTexture(unsigned int shaderProgram, unsigned int* texture);
}