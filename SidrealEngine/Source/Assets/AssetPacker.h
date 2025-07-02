#pragma once
#include <Entity/EntityManager.h>

namespace AssetPacker
{
	enum class AssetType
	{
		Texture,
		Audio,
		Model,
		Shader,
		Animation,
		Font,
		Miscellaneous
	};

	void AddAssetToPack(const char* assetPath, AssetType type);
	void AddSceneToPack(const char* scenePath);
	void PackAllAssets();
	void LoadAllPackedAssets(const char* path, EntityManager* entityManager);
	bool LoadPackedModelByName(const char* path, const char* name, Model& model, Entity entity);
}