#pragma once

#include <vector>
#include "Renderer/Model.h"

namespace Scene
{
	struct SceneData
	{
		std::vector<Model*> models;
		unsigned int skyboxTexture;
	};

	SceneData* GetActiveScene();
	void SetActiveScene(SceneData* scene);
}