#include "AssetPacker.h"
#include <iostream>
#include <vector>
#include <Entity/EntityManager.h>
#include <json.hpp>
#include <fstream>
#include "ModelLoader.h"


const static char SidrealAssetPackHeader[] = "SIDREAL_ENGINE_ASSET_PACK";
void SerializeModel(const Model& model, std::vector<char>& out);
Model DeserializeModel(const char* data, size_t size);

struct AssetEntry
{
	char name[256];     // Asset name or path
	uint32_t offset;    // Offset into the data section
	uint32_t size;      // Size in bytes
	uint32_t type;      // Asset type
};

struct AssetData
{
	AssetEntry entry = {};       // Metadata for the asset
	std::vector<char> data = {}; // Pointer to the raw asset data
};

std::unordered_map<std::string, AssetData> assetMap;
std::unordered_map<std::string, AssetData> loadedAssetMap;

void AssetPacker::AddAssetToPack(const char* assetPath, AssetType type)
{
	std::cout << "Adding asset to pack:" << assetPath << "\n";

	// Check if the asset path is valid
	if (assetPath == nullptr || assetPath[0] == '\0') {
		std::cerr << "Invalid asset path provided.\n";
		return;
	}

	// Check if we already have this asset in the pack
	if (assetMap.find(assetPath) != assetMap.end()) {
		std::cerr << "Asset already exists in the pack: " << assetPath << "\n";
		return;
	}

	// Check if file exists
	FILE* file = fopen(assetPath, "rb");
	if (!file) {
		std::cerr << "Failed to open asset file: " << assetPath << "\n";
		return;
	}

	if (type == AssetType::Model) {
		Model model;
		ModelLoader::LoadModel(assetPath, model);

		// Add model to asset map
        AssetData assetData{};
		strncpy(assetData.entry.name, assetPath, sizeof(assetData.entry.name) - 1);
		assetData.entry.name[sizeof(assetData.entry.name) - 1] = '\0'; // Ensure null termination
		assetData.entry.offset = 0; // Offset will be set later
		assetData.entry.type = static_cast<uint32_t>(type);

		SerializeModel(model, assetData.data);

		assetData.entry.size = static_cast<uint32_t>(assetData.data.size());
		assetMap[assetPath] = assetData;
	}
}

void AssetPacker::AddSceneToPack(const char* scenePath)
{
	using json = nlohmann::json;

	// Check if path is valid
	if (scenePath == nullptr) {
		std::cerr << "Invalid path." << std::endl;
		return;
	}

	// Check if file exists and try to open
	std::ifstream file(scenePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file." << std::endl;
		return;
	}

	// Read the contents of the file into a string
	std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Parse the JSON
	json j = json::parse(jsonString);
	std::string version = j["Version"];

	// Load all models
	for (json::iterator it = j["Scene"]["Models"].begin(); it != j["Scene"]["Models"].end(); ++it)
	{
		std::string path = it.value()["Path"];
		AssetPacker::AddAssetToPack(path.c_str(), AssetPacker::AssetType::Model);
	}
}

void AssetPacker::PackAllAssets()
{
    std::cout << "Packing all assets...\n";

    FILE* packFile = fopen("assets.sap", "wb");
    if (!packFile)
    {
        std::cerr << "Failed to create asset pack file.\n";
        return;
    }

    // --- Write header ---
    fwrite(SidrealAssetPackHeader, sizeof(char), sizeof(SidrealAssetPackHeader), packFile);

    size_t assetCount = assetMap.size();
    fwrite(&assetCount, sizeof(size_t), 1, packFile);

    // Placeholder for asset entries
    long entryTablePos = ftell(packFile);
    for (size_t i = 0; i < assetCount; ++i)
    {
        AssetEntry dummy = {};
        fwrite(&dummy, sizeof(AssetEntry), 1, packFile);
    }

    // --- Write asset data and fill in actual entries ---
    std::vector<AssetEntry> entries;
    for (auto& [name, assetData] : assetMap)
    {
		AssetEntry& entry = assetData.entry;
		entry.offset = ftell(packFile); // Current position is the offset
		entry.size = static_cast<uint32_t>(assetData.data.size()); // Size of the asset data
		entries.push_back(entry);

		// Write the asset data to the pack file
		fwrite(assetData.data.data(), 1, assetData.data.size(), packFile);
    }

    // --- Go back and write metadata entries ---
    fseek(packFile, entryTablePos, SEEK_SET);
    for (const auto& entry : entries)
    {
        fwrite(&entry, sizeof(AssetEntry), 1, packFile);
    }

    fclose(packFile);
    std::cout << "Asset packing complete. " << entries.size() << " assets packed.\n";
}

void AssetPacker::LoadAllPackedAssets(const char* path, EntityManager* entityManager)
{
	// Check if the asset path is valid
	if (path == nullptr || path[0] == '\0') {
		std::cerr << "Invalid asset path provided.\n";
		return;
	}

	loadedAssetMap.clear();

	FILE* file = fopen(path, "rb");
	if (!file)
	{
		std::cerr << "Failed to open Sidreal Asset Pack from location: " << path << "\n";
	}

	// --- Read Header ---
	const size_t headerSize = sizeof(SidrealAssetPackHeader);
	char header[headerSize] = {};
	size_t readBytes = fread(header, 1, headerSize, file);

	if (readBytes != sizeof(SidrealAssetPackHeader))
	{
		std::cerr << "Failed to read asset pack header.\n";
		fclose(file);
		return;
	}

	size_t assetCount = 0;
	fread(&assetCount, sizeof(size_t), 1, file);

	if (assetCount == 0)
	{
		std::cerr << "No assets found in the pack.\n";
		fclose(file);
		return;
	}

	// --- Reas Asset Metadata ---
	std::vector<AssetEntry> assetMap;
	assetMap.resize(assetCount);
	size_t entriesRead = fread(assetMap.data(), sizeof(AssetEntry), assetCount, file);
	if (entriesRead != assetCount)
	{
		std::cerr << "Failed to read asset entries from pack.\n";
		fclose(file);
		return;
	}

	std::cout << "Loaded " << assetCount << " assets from pack.\n";

	for (const AssetEntry& entry : assetMap)
	{
		fseek(file, entry.offset, SEEK_SET);

		std::vector<char> data(entry.size);
		fread(data.data(), 1, entry.size, file);

		AssetData asset;
		asset.entry = entry;
		asset.data = std::move(data);

		loadedAssetMap[std::string(entry.name)] = std::move(asset);
	}

	const auto& assetData = loadedAssetMap["\\cube.glb"];


	//Entity entity = entityManager->CreateEntity();
	entityManager->hasTransform[0] = true;
	EntityTransform::Transform& transform = entityManager->transforms[0];
	transform.position = { 0.0f, 0.0f, 3.0f };
	transform.rotation = { 0.0f, 0.0f, 0.0f };
	transform.scale = { 1.0f, 1.0f, 1.0f };

	// Load all models into the entity manager
	// Get everyhting from unordered map
	for (const auto& [name, assetData] : loadedAssetMap)
	{
		if (assetData.entry.type == static_cast<uint32_t>(AssetType::Model))
		{
			entityManager->hasModel[0] = true;
			Model& model = entityManager->models[0];
			model = DeserializeModel(assetData.data.data(), assetData.data.size());
			ModelLoader::SetupModelOpenGL(model.meshes);
		}
	}
}

bool AssetPacker::LoadPackedModelByName(const char* path, const char* name, Model& model)
{
	// Check if the asset path is valid
	if (path == nullptr || path[0] == '\0') {
		std::cerr << "Invalid asset path provided.\n";
		return false;
	}

	FILE* file = fopen(path, "rb");
	if (!file)
	{
		std::cerr << "Failed to open Sidreal Asset Pack from location: " << path << "\n";
		return false;
	}

	// --- Read Header ---
	const size_t headerSize = sizeof(SidrealAssetPackHeader);
	char header[headerSize] = {};
	size_t readBytes = fread(header, 1, headerSize, file);

	if (readBytes != sizeof(SidrealAssetPackHeader))
	{
		std::cerr << "Failed to read asset pack header.\n";
		fclose(file);
		return false;
	}

	size_t assetCount = 0;
	fread(&assetCount, sizeof(size_t), 1, file);

	if (assetCount == 0)
	{
		std::cerr << "No assets found in the pack.\n";
		fclose(file);
		return false;
	}

	// --- Read Asset Metadata ---
	std::vector<AssetEntry> assetMap;
	assetMap.resize(assetCount);
	size_t entriesRead = fread(assetMap.data(), sizeof(AssetEntry), assetCount, file);
	if (entriesRead != assetCount)
	{
		std::cerr << "Failed to read asset entries from pack.\n";
		fclose(file);
		return false;
	}

	// --- Find the asset by name ---
	AssetData assetData;
	for (const AssetEntry& entry : assetMap)
	{
		if (std::string(entry.name) != std::string(name)) continue;

		fseek(file, entry.offset, SEEK_SET);

		std::vector<char> data(entry.size);
		fread(data.data(), 1, entry.size, file);

		assetData.entry = entry;
		assetData.data = std::move(data);
	}
	fclose(file);

	// --- Check if we found the asset ---
	if (assetData.entry.name[0] == '\0')
	{
		std::cerr << "Asset not found: " << name << "\n";
		return false;
	}

	// --- Deserialize the model ---
	if (assetData.entry.type == static_cast<uint32_t>(AssetType::Model))
	{
		model = DeserializeModel(assetData.data.data(), assetData.data.size());
		ModelLoader::SetupModelOpenGL(model.meshes);
		std::cout << "Loaded " << assetData.entry.name << "\n";
	}
	else
	{
		std::cerr << "Asset type mismatch. Expected Model, got " << assetData.entry.type << "\n";
		return false;
	}
	
	return true;
}









void SerializeVertex(const Vertex& v, std::vector<char>& out)
{
	out.insert(out.end(), reinterpret_cast<const char*>(&v), reinterpret_cast<const char*>(&v) + sizeof(Vertex));
}

void SerializeMesh(const Mesh& mesh, std::vector<char>& out)
{
	uint32_t vertexCount = static_cast<uint32_t>(mesh.vertices.size());
	out.insert(out.end(), reinterpret_cast<const char*>(&vertexCount), reinterpret_cast<const char*>(&vertexCount) + sizeof(uint32_t));

	for (const Vertex& v : mesh.vertices)
		SerializeVertex(v, out);

	uint32_t indexCount = static_cast<uint32_t>(mesh.indices.size());
	out.insert(out.end(), reinterpret_cast<const char*>(&indexCount), reinterpret_cast<const char*>(&indexCount) + sizeof(uint32_t));
	out.insert(out.end(), reinterpret_cast<const char*>(mesh.indices.data()), reinterpret_cast<const char*>(mesh.indices.data()) + indexCount * sizeof(unsigned int));

	// If you want to store textures, store string names or IDs — NOT the Texture objects themselves.
	uint32_t textureCount = 0;
	out.insert(out.end(), reinterpret_cast<const char*>(&textureCount), reinterpret_cast<const char*>(&textureCount) + sizeof(uint32_t));
}

void SerializeModel(const Model& model, std::vector<char>& out)
{
	// Serialize name length + name
	uint32_t nameLen = static_cast<uint32_t>(model.name.size());
	out.insert(out.end(), reinterpret_cast<const char*>(&nameLen), reinterpret_cast<const char*>(&nameLen) + sizeof(uint32_t));
	out.insert(out.end(), model.name.begin(), model.name.end());

	// Serialize instances and UV tile
	out.insert(out.end(), reinterpret_cast<const char*>(&model.instances), reinterpret_cast<const char*>(&model.instances) + sizeof(unsigned int));
	out.insert(out.end(), reinterpret_cast<const char*>(&model.uvTileFactor), reinterpret_cast<const char*>(&model.uvTileFactor) + sizeof(float));

	// Serialize meshes
	uint32_t meshCount = static_cast<uint32_t>(model.meshes.size());
	out.insert(out.end(), reinterpret_cast<const char*>(&meshCount), reinterpret_cast<const char*>(&meshCount) + sizeof(uint32_t));

	for (const Mesh& mesh : model.meshes)
		SerializeMesh(mesh, out);
}

Model DeserializeModel(const char* data, size_t size)
{
	Model model;
	const char* cursor = data;
	const char* end = data + size;

	auto read = [&](void* dest, size_t sz) {
		if (cursor + sz > end) throw std::runtime_error("DeserializeModel: Buffer overrun.");
		std::memcpy(dest, cursor, sz);
		cursor += sz;
		};

	// Read name
	uint32_t nameLen;
	read(&nameLen, sizeof(uint32_t));

	model.name.resize(nameLen);
	read(model.name.data(), nameLen);

	// Read instances and UV tile
	read(&model.instances, sizeof(unsigned int));
	read(&model.uvTileFactor, sizeof(float));

	// Read mesh count
	uint32_t meshCount;
	read(&meshCount, sizeof(uint32_t));
	model.meshes.resize(meshCount);

	for (uint32_t i = 0; i < meshCount; ++i)
	{
		Mesh& mesh = model.meshes[i];

		// Read vertex count
		uint32_t vertexCount;
		read(&vertexCount, sizeof(uint32_t));
		mesh.vertices.resize(vertexCount);
		read(mesh.vertices.data(), vertexCount * sizeof(Vertex));

		// Read index count
		uint32_t indexCount;
		read(&indexCount, sizeof(uint32_t));
		mesh.indices.resize(indexCount);
		read(mesh.indices.data(), indexCount * sizeof(unsigned int));

		// Skip texture data (or extend this later)
		uint32_t textureCount;
		read(&textureCount, sizeof(uint32_t));
		// Skipping actual texture info for now
	}

	return model;
}