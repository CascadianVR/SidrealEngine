#include <glad/glad.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <json.hpp>
#include "GLTFLoader.h"
#include "Renderer/Model.h"
#include "Texture.h"

using namespace nlohmann;

unsigned int ARRAY_BUFFER = 34962;
unsigned int ELEMENT_ARRAY_BUFFER = 34963;


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

Texture::Texture LoadDefaultTexture();
Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

Model GLTFLoader::LoadBinary(const char* path)
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
	std::cout << "Version: " << version << std::endl;
	std::cout << "Length: " << length << std::endl << std::endl;

	// Check if file size matches length in header
	if (size < length)
	{
		throw std::runtime_error("Invalid .glb file");
	}

	// Get json chunk length starting from 12th byte (after header) as uint32_t
	uint32_t jsonChunkLength = *reinterpret_cast<uint32_t*>(&buffer[12]);

	// Check if json chunk is long enough (20 bytes is header + json chunk length)
	if (size < jsonChunkLength + 20)
	{
		throw std::runtime_error("Invalid .glb file");
	}

	// Get json chunk
	std::string jsonChunk(buffer.begin() + 20, buffer.begin() + 20 + jsonChunkLength);

	// Print json chunk
	std::cout << "JSON chunk: " << jsonChunk << std::endl;

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
	std::cout << "Accessors size: " << jsonAccessors.size() << std::endl;
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
	std::cout << "BufferViews size: " << jsonBufferViews.size() << std::endl;
	std::cout << "Buffers size: " << jsonData["buffers"].size() << std::endl;

	for (int i = 0; i < accessors.size(); i++)
	{
		json bufferView = jsonBufferViews[accessors[i].BufferView];

		BufferView view;
		view.Buffer = bufferView["buffer"];
		view.ByteLength = bufferView["byteLength"];
		view.ByteOffset = bufferView["byteOffset"];
		view.Target = bufferView["target"];
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

	for (int i = 0; i < accessors.size(); i++)
	{
		std::cout << "BufferView: " << accessors[i].BufferView << std::endl;
		std::cout << "ComponentType: " << accessors[i].CompType << std::endl;
		std::cout << "Count: " << accessors[i].Count << std::endl;
		std::cout << "Type: " << accessors[i].Type << std::endl;
	}

	// Get number of meshes and "primatives" (primative is the sub-mesh that is contained in a simgle material)
	json jsonMeshes = jsonData["meshes"];
	std::vector<Mesh> meshes;
	unsigned int totalMeshCount = 0;
	for(int i = 0; i < jsonMeshes.size(); i++)
	{
		json jsonMesh = jsonMeshes[i];
		json jsonPrimitives = jsonMesh["primitives"];
		totalMeshCount += jsonPrimitives.size();
	}

	// Load each mesh by finding the vertex data and index data from the buffer data using the offsets
	for (int i = 0; i < totalMeshCount; i++)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		/* ---------- GET VERTICES ---------- */
		// Get vertex position byte offset
		BufferView view = bufferViews[0 + i * 4];
		std::vector<char> bufferData = buffers[view.Buffer];
		uint32_t positionByteOffset = view.ByteOffset;

		// Get vertex normal byte offset
		view = bufferViews[1 + i * 4];
		uint32_t normalByteOffset = view.ByteOffset;

		// Get vertex texCoords byte offset
		view = bufferViews[2 + i * 4];
		uint32_t texCoordsByteOffset = view.ByteOffset;

		for (int j = 0; j < accessors[0 + i * 4].Count; j++)
		{
			Vertex vertex;

			// Get position
			vertex.Position = *reinterpret_cast<glm::vec3*>(&bufferData[positionByteOffset]);
			positionByteOffset += 12;

			// Get normal
			vertex.Normal = *reinterpret_cast<glm::vec3*>(&bufferData[normalByteOffset]);
			normalByteOffset += 12;

			// Get texCoords
			vertex.TexCoords = *reinterpret_cast<glm::vec2*>(&bufferData[texCoordsByteOffset]);
			texCoordsByteOffset += 8;

			vertices.push_back(vertex);
		}

		/* ---------- GET INDICES ---------- */
		// Get indices bufferView
		view = bufferViews[3 + i * 4];
		uint32_t byteOffset = view.ByteOffset;

		std::cout << "Index Byte offset: " << byteOffset << std::endl;

		for (int j = 0; j < accessors[3 + i * 4].Count; j++)
		{
			unsigned int index = *reinterpret_cast<unsigned short*>(&bufferData[byteOffset]);
			byteOffset += 2;
			indices.push_back(index);
		}

		Mesh mesh = CreateMesh(vertices, indices);
		meshes.push_back(mesh);
	}

	Model model;
	model.meshes = meshes;

	return model;
}

Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
	// Create VBOs and IBOs
	std::vector<unsigned int> VAOs(1);
	std::vector<unsigned int> VBOs(1);
	std::vector<unsigned int> EBOs(1);

	// Generate VAO, VBOs, and IBOs
	glGenVertexArrays(1, VAOs.data());
	glGenBuffers(1, VBOs.data());
	glGenBuffers(1, EBOs.data());

	glBindVertexArray(VAOs[0]);

	// Bind and fill VBO with vertex data
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// Bind and fill EBO with indices data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::vector<Mesh> meshes;
	Mesh mesh;

	std::vector<Texture::Texture> textures;
	textures.push_back(LoadDefaultTexture());
	mesh.textures = textures;
	mesh.vertices = vertices;
	mesh.indices = indices;
	mesh.VAO = VAOs[0];
	mesh.VBO = VBOs[0];
	mesh.EBO = EBOs[0];

	return mesh;
}