#pragma once
#include "Model.h"
#include "../Entity/Components/Transform.h"

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
	void RenderModel(Model& model, EntityTransform::Transform& transform, PassType passType);
	void LoadShaders(bool reload);

	void SetLightDirection(float*);
	float* GetLightDirection();

	void ShowDepthMapDebug();
}