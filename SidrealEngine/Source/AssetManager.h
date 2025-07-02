#pragma once
#include <unordered_map>
#include "Renderer/Model.h"
#include "Scene.h"
#include "Entity/EntityManager.h"

namespace AssetManager
{
	Scene::SceneData* LoadSceneFromJSON(const char* jsonPath, EntityManager* entityManager);
}