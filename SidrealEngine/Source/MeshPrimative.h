#pragma once
#include "Renderer/Model.h"
#include "Entity/Components/RenderData.h"

using namespace Components;

namespace MeshPrimative
{
	void CreateCube(Model& skyboxModel, RenderData& skyboxRenderData);
	Model CreateQuad();
	Model CreateSphere();
}