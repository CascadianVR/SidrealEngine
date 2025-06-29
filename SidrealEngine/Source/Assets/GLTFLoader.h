#pragma once
#include "Renderer/Model.h"

namespace GLTFLoader
{
	void LoadBinary(const char* path, Model& model);
	void LoadASCII(const char* path, Model& model);
}