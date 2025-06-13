#pragma once
#include <unordered_map>
#include "Renderer/Model.h"
#include "Scene.h"
#include "Entity/EntityManager.h"

namespace AssetManager
{
	Scene::SceneData* LoadSceneFromJSON(const char* jsonPath, EntityManager* entityManager);
	std::unordered_map<std::string, Model>* GetModels();
	Model* GetModel(const std::string& name);
	void AddModel(const std::string& name, const Model& model);
}