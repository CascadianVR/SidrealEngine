#pragma once
#include "Renderer/Model.h"

namespace ModelLoader
{
	void LoadModel(const char* path, Model& model);
	void SetupModelOpenGL(std::vector<Mesh>& meshes);
}