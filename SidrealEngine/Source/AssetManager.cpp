#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>
#include "AssetManager.h"
#include "ModelLoader.h"
#include "Renderer/Model.h"
#include "Scene.h"
#include "Entity/EntityManager.h"
#include "Entity/Components/Transform.h"

Scene::SceneData* AssetManager::LoadSceneFromJSON(const char* jsonPath, EntityManager* entityManager)
{
	using json = nlohmann::json;

	// Check if path is valid
	if (jsonPath == nullptr) {
		std::cerr << "Invalid path." << std::endl;
		return nullptr;
	}

	// Check if file exists and try to open
	std::ifstream file(jsonPath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file." << std::endl;
		return nullptr;
	}

	// Read the contents of the file into a string
	std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Parse the JSON
	json j = json::parse(jsonString);
	std::string version = j["Version"];
	std::cout << "Version: " << version << std::endl;

	Scene::SceneData* scene = new Scene::SceneData();

	// Load skybox texture
	std::string path = j["Scene"]["Skybox"]["Path"];
	unsigned int skyboxTex = Texture::CreateTextureHDR(path.c_str());
	scene->skyboxTexture = skyboxTex;

	// Load all models
	for (json::iterator it = j["Scene"]["Models"].begin(); it != j["Scene"]["Models"].end(); ++it)
	{
		Entity entity = entityManager->CreateEntity();
		entityManager->hasTransform[entity] = true;
		EntityTransform::Transform& transform = entityManager->transforms[entity];

		std::string path = it.value()["Path"];
		json value = it.value()["Position"];
		if (!value.is_null())
			transform.position = glm::vec3(value[0], value[1], value[2]);

		value = it.value()["Rotation"];
		if (!value.is_null())
			transform.rotation = glm::vec3(value[0], value[1], value[2]);

		value = it.value()["Scale"];
		if (!value.is_null())
			transform.scale = glm::vec3(value[0], value[1], value[2]);

		entityManager->hasModel[entity] = true;
		Model& model = entityManager->models[entity];
		ModelLoader::LoadModel(path.c_str(), model);

		value = it.value()["UVTileFactor"];
		if (!value.is_null())
			model.uvTileFactor = value;
	}

	std::cout << "All models loaded!\n\n";

	return scene;
}