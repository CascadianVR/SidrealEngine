#include "AssetLoader.h"
#include <fstream>
#include <iostream>
#include <json.hpp>
#include "ModelLoader.h"

using json = nlohmann::json;
std::vector<ModelLoader::Model> loadedModels;

std::vector<ModelLoader::Model>& AssetLoader::LoadAllAssets()
{
    // Open the JSON file
    std::ifstream file("Resources\\SceneConfig.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
    }

    // Read the contents of the file into a string
    std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::cout << jsonString << std::endl;

    // Parse the JSON
	json j = json::parse(jsonString);
    std::string version = j["Version"];
	std::cout << "Version: " << version << std::endl;

	// Load all models
	for (json::iterator it = j["Scene"].begin(); it != j["Scene"].end(); ++it)
	{
		ModelLoader::Model model;

		std::string path = it.value()["Path"];
		model = ModelLoader::LoadModel(path.c_str());

		json value = it.value()["Position"];
		if (!value.is_null())
			model.position = glm::vec3(value[0], value[1], value[2]);
		
		value = it.value()["Rotation"];
		if (!value.is_null())
			model.rotation = glm::vec3(value[0], value[1], value[2]);
		
		value = it.value()["Scale"];
		if (!value.is_null())
			model.scale = glm::vec3(value[0], value[1], value[2]);
		
		value = it.value()["UVTileFactor"];
		if (!value.is_null())
			model.uvTileFactor = value;

		
		loadedModels.push_back(model);
	}

	return loadedModels;
}