#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/matrix_common.hpp>
#include "../Texture.h"

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
	unsigned int instanceVBO;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture::Texture> textures;
	std::vector<glm::mat4> instanceMatrices = std::vector<glm::mat4>(0);
};

struct Model
{
	std::string name;
	std::vector<Mesh> meshes;
	unsigned int instances = 1;
	float uvTileFactor = 1.0f;
};