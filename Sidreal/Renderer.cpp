#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MeshPrimative.h"
#include "Camera.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "ModelLoader.h"
#include "MathUtils.h"
//nclude "MathUtils.h"

char* LoadShader(const char* path);

unsigned int shaderProgram;
unsigned int* textures;

double lastTime;
double counter;
unsigned int currentTexture;

unsigned int VAO;

std::vector<Model> models;

void Renderer::Initialize()
{
    Camera::Initialize();

    shaderProgram = Shader::CreateShaderProgram("lighting.vert", "lighting.frag");


    models.push_back(MeshPrimative::CreateCube());
    models.push_back(LoadModel("Resources\\CasOC\\CASCAS.obj"));
    models.push_back(LoadModel("Resources\\CUBE.obj"));
    models.push_back(LoadModel("Resources\\Office.obj"));
    models.push_back(LoadModel("Resources\\sphere.obj"));

    models[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    models[0].rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    models[1].position = glm::vec3(2.0f, 0.0f, 0.0f);
    models[1].rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    models[2].position = glm::vec3(4.0f, 0.0f, 0.0f);
    models[2].rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    models[3].position = glm::vec3(-8.0f, 0.0f, 0.0f);
    models[3].rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    models[4].position = glm::vec3(-2.5f, 0.0f, 0.0f);

    // Load texture
    //textures = new unsigned int[5];
    //glActiveTexture(GL_TEXTURE0);
    //textures[0] = Texture::CreateTexture("Resources\\wahh.png");
    //glActiveTexture(GL_TEXTURE1);
    //textures[1] = Texture::CreateTexture("Resources\\wahh.png");
    //glActiveTexture(GL_TEXTURE2);
    //textures[2] = Texture::CreateTexture("Resources\\hedgie.png");
    //glActiveTexture(GL_TEXTURE3);
    //textures[3] = Texture::CreateTexture("Resources\\sipsip.png");
    //glActiveTexture(GL_TEXTURE4);
    //textures[4] = Texture::CreateTexture("Resources\\emoteYES.png");

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Renderer::Render()
{
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;
    counter += delta;

    glClearColor(0.0f, 1.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera::UpdateCamera(&shaderProgram);

    // Set directional light position
    glm::vec3 lightPosz = glm::vec3(glm::cos(glfwGetTime()) * 5, 1.0f, glm::sin(glfwGetTime())*5);
    Shader::SetShaderUniformVec3(&shaderProgram, "lightPos", MathUtils::Vec3toFloat3(lightPosz));

    // Loop through models and draw them
    for (int i = 0; i < models.size(); i++)
    {
        Model model = models[i];
        for (int j = 0; j < model.meshes.size(); j++)
        {
            Mesh mesh = models[i].meshes[j];

            glBindVertexArray(mesh.VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, model.position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0, 0, 1));

            Shader::SetShaderUniformgMatrix4fv(&shaderProgram, "model", modelMatrix);

            float* forward = MathUtils::Vec3toFloat3(Camera::GetCameraForward());
            Shader::SetShaderUniformVec3(&shaderProgram, "cameraForward", forward);

            if (mesh.textures.size() > 0)
            {
              Texture::SetActiveTexture(&shaderProgram, &mesh.textures[0].id, mesh.textures[0].index);
            }

		    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
		}
	}
}

unsigned int* Renderer::GetShaderProgram()
{
	return &shaderProgram;
}