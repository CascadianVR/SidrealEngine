
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <unordered_map>
#include "ModelLoader.h"
#include "Renderer/Model.h"
#include "Texture.h"

using namespace ModelLoader;

void ProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>* meshes);
Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
std::vector<Texture::Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

const char* currentModelPath = nullptr;
unsigned int textureIndex = 0;
std::vector<Texture::Texture> loadedTextures;
std::unordered_map<std::string, Model> loadedModels;
Assimp::Importer importer;

void ModelLoader::LoadModel(const char* path, Model& model)
{
	currentModelPath = path;
    // Get name of file only
    std::string name = std::string(path);
    size_t found = name.find_last_of("/\\");
    if (found != std::string::npos)
        name = name.substr(found + 1);
    std::cout << "--- Loading Model: " << name << "\n";

    //if (AssetManager::GetModel(path) != nullptr && !doNotInstance)
    //{
    //    std::cout << "---- Model already loaded: " << path << std::endl;
    //    std::cout << "---- Generating instance... " << std::endl;
    //
	//	Model* model = AssetManager::GetModel(path);
    //
	//	// Create a new instance of the model and calculate the model matrix
    //    glm::mat4 modelMatrix = glm::mat4(1.0f);
    //    modelMatrix = glm::translate(modelMatrix, transform.position);
    //    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.x), glm::vec3(1, 0, 0));
    //    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.y), glm::vec3(0, 1, 0));
    //    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.z), glm::vec3(0, 0, 1));
    //    modelMatrix = glm::scale(modelMatrix, transform.scale);
    //
	//	// Push the model matrix to each mesh's instance matrices and update the instance VBO data
	//	for (auto& mesh : model->meshes)
	//	{
    //        mesh.instanceMatrices.push_back(modelMatrix);
    //
	//		glBindVertexArray(mesh.VAO);
    //        glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVBO);
    //        glBufferData(GL_ARRAY_BUFFER, mesh.instanceMatrices.size() * sizeof(glm::mat4), &mesh.instanceMatrices[0], GL_STATIC_DRAW);
	//	}
    //
	//	// Increment instance count since we made a new instance
    //    model->instances++;
    //
    //    return model;
	//}

    unsigned int importOptions = aiProcess_Triangulate;

	const aiScene* scene = importer.ReadFile(path, importOptions);
	
    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    std::string stringPath = path;
    std::string directory = stringPath.substr(0, stringPath.find_last_of('/'));

    std::vector<Mesh> meshes;

    ProcessNode(scene->mRootNode, scene, &meshes);

    unsigned int *VAOs, *VBOs, *EBOs, *instanceVBOs;
    VAOs = new unsigned int[meshes.size()];
    VBOs = new unsigned int[meshes.size()];
    EBOs = new unsigned int[meshes.size()];
    instanceVBOs = new unsigned int[meshes.size()];
    glGenVertexArrays(meshes.size(), VAOs);
    glGenBuffers(meshes.size(), VBOs);
    glGenBuffers(meshes.size(), EBOs);
    glGenBuffers(meshes.size(), instanceVBOs);

    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i].VAO = VAOs[i];
        meshes[i].VBO = VBOs[i];
        meshes[i].EBO = EBOs[i];
        meshes[i].instanceVBO = instanceVBOs[i];

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


		//glm::mat4 modelMatrix = glm::mat4(1.0f);
		//modelMatrix = glm::translate(modelMatrix, transform.position);
		//modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.x), glm::vec3(1, 0, 0));
		//modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.y), glm::vec3(0, 1, 0));
		//modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.z), glm::vec3(0, 0, 1));
		//modelMatrix = glm::scale(modelMatrix, transform.scale);
        //
		//meshes[i].instanceMatrices.push_back(modelMatrix);
        //
        //
        //glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);
        //glBufferData(GL_ARRAY_BUFFER, meshes[i].instanceMatrices.size() * sizeof(glm::mat4), &meshes[i].instanceMatrices[0], GL_STATIC_DRAW);
        //
        //glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        //glEnableVertexAttribArray(3);
        //
        //glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        //glEnableVertexAttribArray(4);
        //
        //glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        //glEnableVertexAttribArray(5);
        //
        //glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
        //glEnableVertexAttribArray(6);
        //
        //glVertexAttribDivisor(3, 1);
        //glVertexAttribDivisor(4, 1);
        //glVertexAttribDivisor(5, 1);
        //glVertexAttribDivisor(6, 1);
	}

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    model.meshes = meshes;
	model.name = name;
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
        Vertex vertex{};
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

    // Load all textures of a given type
    for (unsigned int i = 0; i < aiMesh->mMaterialIndex; i++)
    {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
		std::vector<Texture::Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    }

    // Load default texture if no material was loaded
    if (aiMesh->mMaterialIndex <= 0)
    {
        aiMaterial* material = new aiMaterial();
        std::vector<Texture::Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	}

    //std::cout << "Material Index: " << aiMesh->mMaterialIndex << std::endl;
    //
    //for (unsigned int i = 0; i < aiMesh->mNumBones; i++)
    //{
    //	aiBone* bone = aiMesh->mBones[i];
    //    std::cout << "Bone: " << bone->mName.C_Str() << std::endl;
    //}

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
        
			// Get directory of model
			std::string path = std::string(currentModelPath);
            size_t found = path.find_last_of("/\\");
            if (found != std::string::npos)
            {
                path = path.substr(0, found);
            }

            path.append("\\");
            path.append(texturePath.C_Str());
        
            texture.id = Texture::CreateTexture2D(path.c_str());
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

            texture.id = Texture::CreateTexture2D("Resources\\default.png");
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