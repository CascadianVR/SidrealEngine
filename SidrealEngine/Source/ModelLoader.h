#pragma once
#include "Renderer/Model.h"
#include <Entity/EntityManager.h>

namespace ModelLoader
{
	void LoadModel(const char* path, Model& model);
	void SetupModelOpenGL(std::vector<Mesh>& meshes, Entity entity);
}