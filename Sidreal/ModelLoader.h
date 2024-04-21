#pragma once
#include "Texture.h"
#include <iostream>
#include <vector>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glad/glad.h>

typedef struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

typedef struct Mesh
{
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture::Texture> textures;
};

typedef struct Model
{
	std::vector<Mesh> meshes;
	glm::vec3 position;
	glm::vec3 rotation;
};

Model LoadModel(const char* path);