#pragma once
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include "Texture.h"

namespace ModelLoader
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	struct Mesh
	{
		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture::Texture> textures;
	};

	struct Model
	{
		std::vector<Mesh> meshes;
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
		float uvTileFactor = 1.0f;
	};

	Model LoadModel(const char* path);
}