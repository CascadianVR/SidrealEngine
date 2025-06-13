#pragma once
#include "EntityManager.h"
#include "Components/Transform.h"
#include "../Renderer/Model.h"

void RenderSystem(EntityManager& entityManager)
{
	ScopedTimer timer("RenderSystem");

	// Clear the screen
    glClearColor(0.0f, 1.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        // Render shadow pass
		ScopedTimer timer("ShadowPass");
        Renderer::SetupShadowPass();
        for (int i = 0; i < MAX_ENTITIES; ++i) {
            if (entityManager.hasTransform[i] && entityManager.hasModel[i]) {
                EntityTransform::Transform& transform = entityManager.transforms[i];
                Model& model = entityManager.models[i];
                Renderer::RenderModel(model, transform, Renderer::PassType::Shadow);
            }
        }
    }

    {
        // Render lighting pass
        ScopedTimer timer("LightingPass");
        Renderer::SetupLightingPass();
        for (int i = 0; i < MAX_ENTITIES; ++i) {
            if (entityManager.hasTransform[i] && entityManager.hasModel[i]) {
                EntityTransform::Transform& transform = entityManager.transforms[i];
                Model& model = entityManager.models[i];

                Renderer::RenderModel(model, transform, Renderer::PassType::Lighting);
            }
        }
    }

    Renderer::ShowDepthMapDebug();
}