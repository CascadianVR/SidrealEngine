#pragma once
#include "Entity/Components/Transform.h"
#include "Entity/Components/RenderData.h"
#include "Model.h"

namespace Renderer
{
	enum class PassType
	{
		Shadow = 0,
		Lighting = 1,
	};

	void Initialize();
	void SetupShadowPass();
	void SetupLightingPass();
	void RenderModel(Model& model, Components::Transform& transform, Components::RenderData renderData, PassType passType);
	void LoadShaders(bool reload);

	void SetLightDirection(float*);
	float* GetLightDirection();

	void ShowDepthMapDebug();
}