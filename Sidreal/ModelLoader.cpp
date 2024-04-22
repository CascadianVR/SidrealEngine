
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "ModelLoader.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glad/glad.h>
#include "Texture.h"

void ProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>* meshes);
Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
std::vector<Texture::Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

unsigned int textureIndex = 0;
std::vector<Texture::Texture> loadedTextures;

Model LoadModel(const char* path)
{
	Assimp::Importer importer;

    unsigned int importOptions = aiProcess_Triangulate;

	const aiScene* scene = importer.ReadFile(path, importOptions);
	
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    }

    std::string stringPath = path;
    std::string directory = stringPath.substr(0, stringPath.find_last_of('/'));

    std::vector<Mesh> meshes;

    ProcessNode(scene->mRootNode, scene, &meshes);

    unsigned int *VAOs, *VBOs, *EBOs;
    VAOs = new unsigned int[meshes.size()];
    VBOs = new unsigned int[meshes.size()];
    EBOs = new unsigned int[meshes.size()];
    glGenVertexArrays(meshes.size(), VAOs);
    glGenBuffers(meshes.size(), VBOs);
    glGenBuffers(meshes.size(), EBOs);

    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i].VAO = VAOs[i];
        meshes[i].VBO = VBOs[i];
        meshes[i].EBO = EBOs[i];

        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);

        // Bind and fill VBO with vertex data
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, meshes[i].vertices.size() * sizeof(Vertex), meshes[i].vertices.data(), GL_STATIC_DRAW);

		// Bind and fill EBO with indices data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshes[i].indices.size() * sizeof(unsigned int), meshes[i].indices.data(), GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// Texture attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    Model model;
    model.meshes = meshes;

    return model;
}

void ProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>* meshes)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes->push_back(ProcessMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, meshes);
    }
}

Mesh ProcessMesh(aiMesh* aiMesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture::Texture> textures;

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position = glm::vec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
        vertex.Normal = glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
        vertex.TexCoords = glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
		aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
			indices.push_back(face.mIndices[j]);
		}
	}

    for (unsigned int i = 0; i < aiMesh->mMaterialIndex; i++)
    {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
		std::vector<Texture::Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    }

    if (aiMesh->mMaterialIndex <= 0)
    {
        aiMaterial* material = new aiMaterial();
        std::vector<Texture::Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	}

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.textures = textures;

    return mesh;
}

std::vector<Texture::Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture::Texture> textures;

    // Load all textures of a given type
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
		aiString texturePath;
		mat->GetTexture(type, i, &texturePath);

        // Check if texture was already loaded
        bool textureLoaded = false;
        for (int j = 0; j < loadedTextures.size(); j++)
        {
            if (loadedTextures[j].path == texturePath.C_Str())
            {
                textureLoaded = true;
                textures.push_back(loadedTextures[j]);
                textureIndex++;

                break;
            }
        }

        // Load texture if not already loaded
        if (!textureLoaded)
        {
            Texture::Texture texture;
        
            std::string path = "Resources\\CasOC\\";
            path.append(texturePath.C_Str());
        
            texture.id = Texture::CreateTexture(path.c_str(), textureIndex);
            texture.index = textureIndex;
            texture.type = typeName;
            texture.path = texturePath.C_Str();
            textures.push_back(texture);

            loadedTextures.push_back(texture);

            textureIndex++;
        }
	}

    // Load default texture if no texture was loaded
    if (mat->GetTextureCount(type) <= 0) 
    {    
        // Check if texture was already loaded
        bool textureLoaded = false;
        for (int j = 0; j < loadedTextures.size(); j++)
        {
            if (loadedTextures[j].path == "Resources\\default.png")
            {
                textureLoaded = true;
                textures.push_back(loadedTextures[j]);
                textureIndex++;
                break;
            }
        }
    
        // Load texture if not already loaded
        if (!textureLoaded)
        {
            Texture::Texture texture;

            texture.id = Texture::CreateTexture("Resources\\default.png", textureIndex);
            texture.index = textureIndex;
            texture.type = typeName;
            texture.path = "Resources\\default.png";
            textures.push_back(texture);

            loadedTextures.push_back(texture);

            textureIndex++;
        }
    }

	return textures;
}