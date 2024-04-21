#pragma once

namespace Texture
{
	typedef struct
	{
		unsigned int id;
		const char* type;
		const char* path;
	} Texture;

	unsigned int CreateTexture(const char* path);
	unsigned int CreateBindlessTexture(const char* path);
	void SetActiveTexture(unsigned int* shaderProgram, unsigned int* texture);
	void SetActiveBindlessTexture(unsigned int* shaderProgram, unsigned int* texture);
}