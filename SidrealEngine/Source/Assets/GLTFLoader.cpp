#include <fstream>
#include <vector>
#include <iostream>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <json.hpp>
#include "Assets/GLTFLoader.h"
#include "Renderer/Model.h"
#include "Texture.h"

using namespace nlohmann;

unsigned int ARRAY_BUFFER = 34962;
unsigned int ELEMENT_ARRAY_BUFFER = 34963;

const char* ReadBufferData(int accessorIndex, json& jsonAccessors, json& jsonBufferViews, std::vector<std::vector<char>>& buffers);

enum class ComponentType : uint32_t
{
	BYTE = 5120,
	UNSIGNED_BYTE = 5121,
	SHORT = 5122,
	UNSIGNED_SHORT = 5123,
	UNSIGNED_INT = 5125,
	FLOAT = 5126
};

struct Accessor
{
	uint32_t BufferView;
	uint32_t CompType;
	uint32_t Count;
	std::string Type;
};

struct BufferView
{
	uint32_t Buffer;
	uint32_t ByteLength;
	uint32_t ByteOffset;
	uint32_t Target;
};

void GLTFLoader::LoadBinary(const char* path, Model& model)
{
	// Open file
	std::ifstream file(path, std::ios::binary | std::ios::ate);

	// Check if file is open
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	// Get file size
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read file into buffer
	std::vector<char> buffer(size);
	file.read(buffer.data(), size);

	// Close file
	file.close();

	// Parse .glb file
	if (size < 12)
	{
		throw std::runtime_error("Invalid .glb file");
	}

	// Check if file is a .glb file
	if (buffer[0] != 'g' || buffer[1] != 'l' || buffer[2] != 'T' || buffer[3] != 'F')
	{
		throw std::runtime_error("Invalid .glb file");
	}

	// Get version and length
	// First 12 bytes are header with magic, version and length (all uint32_t)
	uint32_t version = *reinterpret_cast<uint32_t*>(&buffer[4]);
	uint32_t length = *reinterpret_cast<uint32_t*>(&buffer[8]);

	// Check if version is 2
	if (version != 2)
	{
		throw std::runtime_error("Unsupported version");
	}

	// print version and length
	//std::cout << "Version: " << version << std::endl;
	//std::cout << "Length: " << length << std::endl << std::endl;

	// Check if file size matches length in header
	if (size < length)
	{
		throw std::runtime_error("Invalid .glb file");
	}

	// Get json chunk length starting from 12th byte (after header) as uint32_t
	uint32_t jsonChunkLength = *reinterpret_cast<uint32_t*>(&buffer[12]);

	// Check if json chunk is long enough (20 bytes is header + json chunk length)
	if (size < static_cast<long long>(jsonChunkLength) + 20)
	{
		throw std::runtime_error("Invalid .glb file");
	}

	// Get json chunk
	std::string jsonChunk(buffer.begin() + 20, buffer.begin() + 20 + jsonChunkLength);
	//std::cout << "JSON Chunk: " << jsonChunk << std::endl;

	// Parse json chunk
	json jsonData = nlohmann::json::parse(jsonChunk);

	// Check if json contains buffers
	if (jsonData.find("buffers") == jsonData.end())
	{
		throw std::runtime_error("No buffers found");
	}

	// Check if json contains accessors
	if (jsonData.find("accessors") == jsonData.end())
	{
		throw std::runtime_error("No accessors found");
	}

	// Get accessors
	json jsonAccessors = jsonData["accessors"];

	// Check if accessors is an array or empty
	if (!jsonAccessors.is_array() || jsonAccessors.empty())
	{
		throw std::runtime_error("Accessors is not an array or is empty");
	}

	std::vector<Accessor> accessors;
	for(int i = 0; i < jsonAccessors.size(); i++)
	{
		json accessor = jsonAccessors[i];
		Accessor attribute;
		attribute.BufferView = accessor["bufferView"];
		attribute.CompType = accessor["componentType"];
		attribute.Count = accessor["count"];
		attribute.Type = accessor["type"];
		accessors.push_back(attribute);
	}

	// Get bufferViews
	json jsonBufferViews = jsonData["bufferViews"];

	// Check if json contains bufferViews
	if (jsonData.find("bufferViews") == jsonData.end())
	{
		throw std::runtime_error("No bufferViews found");
	}

	// Check if bufferViews is an array or empty
	if (!jsonBufferViews.is_array() || jsonBufferViews.empty())
	{
		throw std::runtime_error("BufferViews is not an array or is empty");
	}

	std::vector<BufferView> bufferViews;
	for (int i = 0; i < accessors.size(); i++)
	{
		json bufferView = jsonBufferViews[accessors[i].BufferView];

		BufferView view{};
		view.Buffer = bufferView["buffer"];
		view.ByteLength = bufferView["byteLength"];
		view.ByteOffset = bufferView["byteOffset"];
		// Check for optional "target"
		if (bufferView.contains("target"))
		{
			view.Target = bufferView["target"];
		}
		else
		{
			view.Target = -1;
		}
		bufferViews.push_back(view);
	}

	// Get buffers
	json jsonBuffers = jsonData["buffers"];

	// Check if json contains buffers
	if (jsonData.find("buffers") == jsonData.end())
	{
		throw std::runtime_error("No buffers found");
	}

	// Check if buffers is an array or empty
	if (!jsonBuffers.is_array() || jsonBuffers.empty())
	{
		throw std::runtime_error("Buffers is not an array or is empty");
	}

	//Get buffer data using byteLength
	std::vector<std::vector<char>> buffers;
	for(int i = 0; i < jsonBuffers.size(); i++)
	{
		json jsonBuffer = jsonBuffers[i];
		uint32_t byteLength = jsonBuffer["byteLength"];

		// +20 to skip header and +8 to skip buffer chunk header
		std::vector<char> bufferData(buffer.begin() + 20 + jsonChunkLength + 8, buffer.begin() + 20 + jsonChunkLength + 8 + byteLength);
		buffers.push_back(bufferData);
	}

	std::vector<Mesh> meshes;
	json jsonMeshes = jsonData["meshes"];
	json jsonMaterials = jsonData["materials"];
	json jsonTextures = jsonData["textures"];
	json jsonImages = jsonData["images"];

	for (int meshIndex = 0; meshIndex < jsonMeshes.size(); ++meshIndex)
	{
		json meshJson = jsonMeshes[meshIndex];
		json primitives = meshJson["primitives"];

		for (int primIndex = 0; primIndex < primitives.size(); ++primIndex)
		{
			json primitive = primitives[primIndex];
			json attributes = primitive["attributes"];

			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;

			// Read attributes
			int positionAccessorIndex = attributes.value("POSITION", -1);
			if (positionAccessorIndex < 0) {
				std::cerr << "Missing POSITION attribute" << std::endl;
				continue;
			}
			int count = jsonAccessors[positionAccessorIndex].value("count", 0);

			const char* positionData = ReadBufferData(attributes["POSITION"], jsonAccessors, jsonBufferViews, buffers);
			const char* normalData = ReadBufferData(attributes["NORMAL"], jsonAccessors, jsonBufferViews, buffers);
			const char* texCoordData = ReadBufferData(attributes["TEXCOORD_0"], jsonAccessors, jsonBufferViews, buffers);

			for (int i = 0; i < count; ++i)
			{
				Vertex v{};

				// Each vertex entry is tightly packed (assumes float format, 3*4 bytes per vec3, 2*4 bytes per vec2)
				v.Position = *reinterpret_cast<const glm::vec3*>(positionData + i * sizeof(glm::vec3));
				v.Normal = *reinterpret_cast<const glm::vec3*>(normalData + i * sizeof(glm::vec3));
				v.TexCoords = *reinterpret_cast<const glm::vec2*>(texCoordData + i * sizeof(glm::vec2));

				vertices.push_back(v);
			}

			// Read indices
			int indicesAccessorIndex = primitive["indices"];
			json indexAccessor = jsonAccessors[indicesAccessorIndex];
			const char* indexData = ReadBufferData(indicesAccessorIndex, jsonAccessors, jsonBufferViews, buffers);

			int indexCount = indexAccessor["count"];
			int componentType = indexAccessor["componentType"]; // 5123 = ushort, 5125 = uint

			if (componentType == 5123) // UNSIGNED_SHORT
			{
				for (int i = 0; i < indexCount; ++i)
				{
					uint16_t index = *reinterpret_cast<const uint16_t*>(indexData + i * sizeof(uint16_t));
					indices.push_back(static_cast<unsigned int>(index));
				}
			}
			else if (componentType == 5125) // UNSIGNED_INT
			{
				for (int i = 0; i < indexCount; ++i)
				{
					uint32_t index = *reinterpret_cast<const uint32_t*>(indexData + i * sizeof(uint32_t));
					indices.push_back(index);
				}
			}
			else
			{
				std::cerr << "Unsupported index component type: " << componentType << std::endl;
			}

			std::vector<Texture::Texture> textures;
			// Read material
			int materialIndex = primitive.value("material", -1);
			if (materialIndex >= 0) 
			{
				const json& material = jsonMaterials[materialIndex];
				const json& pbr = material["pbrMetallicRoughness"];

				if (pbr.contains("baseColorTexture"))
				{
					int textureIndex = pbr["baseColorTexture"]["index"];
					int imageIndex = jsonTextures[textureIndex]["source"];
					const json& image = jsonImages[imageIndex];

					int bufferViewIndex = image["bufferView"];
					const json& bufferView = jsonBufferViews[bufferViewIndex];

					size_t offset = bufferView.value("byteOffset", 0);
					size_t length = bufferView["byteLength"];
					int bufferIndex = bufferView["buffer"];

					const unsigned char* data = reinterpret_cast<const unsigned char*>(buffers[bufferIndex].data() + offset);
					unsigned int texID = Texture::CreateTexture2D(data, length);

					Texture::Texture texture;
					texture.id = texID;
					texture.index = textureIndex;
					textures.push_back(texture);
				}
			}

			// Final mesh push
			Mesh mesh;
			mesh.vertices = vertices;
			mesh.indices = indices;
			mesh.textures = textures;
			meshes.push_back(mesh);
		}
	}

	model.meshes = meshes;
}

const char* ReadBufferData(int accessorIndex, json& jsonAccessors, json& jsonBufferViews, std::vector<std::vector<char>>& buffers)
{
	json accessor = jsonAccessors[accessorIndex];
	int bufferViewIndex = accessor["bufferView"];
	json bufferView = jsonBufferViews[bufferViewIndex];

	size_t accessorByteOffset = accessor.contains("byteOffset") ? accessor["byteOffset"].get<size_t>() : 0;
	size_t viewByteOffset = bufferView.contains("byteOffset") ? bufferView["byteOffset"].get<size_t>() : 0;

	int bufferIndex = bufferView["buffer"];
	const std::vector<char>& bufferData = buffers[bufferIndex];

	return bufferData.data() + accessorByteOffset + viewByteOffset;
}