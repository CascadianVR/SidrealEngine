#pragma once
#include <string>
#include <vector>

namespace Texture
{
	struct TextureData
	{
		unsigned int width;
		unsigned int  height;
		unsigned int  numChannels;
		std::vector<unsigned char> data;
	};

	TextureData LoadTexture2D(const char* path);
	TextureData CreateTexture2D(const unsigned char* data, size_t dataSize);
	unsigned int InitTexture2D_OpenGL(unsigned char* data, int width, int height, int nrChannels);
	unsigned int LoadTextureHDR(const char* path);
	unsigned int CreateBindlessTexture(const char* path);
	void SetActiveAndBindTexture(unsigned int texture, unsigned int index);
	void SetActiveAndBindTextureArray(unsigned int texture, unsigned int index);
	void SetActiveBindlessTexture(unsigned int shaderProgram, unsigned int* texture);
}