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


Model* models;

void Renderer::Initialize()
{
    Camera::Initialize();

    shaderProgram = Shader::CreateShaderProgram("lighting.vert", "lighting.frag");

    models = new Model[4];

    models[0] = MeshPrimative::CreateCube();
    models[0].position = glm::vec3(4.0f, 0.0f, 0.0f);

    models[1] = LoadModel("Resources\\CasOC\\CasOC.fbx");
    models[1].position = glm::vec3(-3.0f, 0.0f, 0.0f);

    models[2] = LoadModel("Resources\\CUBE.obj");
    models[2].position = glm::vec3(0.0f, 0.0f, 0.0f);
    
    models[3] = LoadModel("Resources\\Office.obj");
    models[3].position = glm::vec3(-6.0f, 0.0f, 0.0f);

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

    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //// Cycle through each texture every second
    //if (counter > 1) {
    //    currentTexture = (currentTexture + 1) % 5;
    //    counter = 0;
    //}
    //Texture::SetActiveTexture(&shaderProgram, &textures[0]);
    Camera::UpdateCamera(&shaderProgram);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < models[i].meshes.size(); j++)
        {
            glBindVertexArray(models[i].meshes[j].VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, models[i].meshes[j].EBO);

            Shader::SetShaderUniformVec3(&shaderProgram, "position", MathUtils::Vec3toFloat3(models[i].position));
            Shader::SetShaderUniformVec3(&shaderProgram, "rotation", MathUtils::Vec3toFloat3(models[i].position));

            float* forward = MathUtils::Vec3toFloat3(Camera::GetCameraForward());
            Shader::SetShaderUniformVec3(&shaderProgram, "cameraForward", forward);

            if (models[i].meshes[j].textures.size() > 0)
            {
                Texture::SetActiveTexture(&shaderProgram, &models[i].meshes[j].textures[0].id);
            }

		    glDrawElements(GL_TRIANGLES, models[i].meshes[j].indices.size(), GL_UNSIGNED_INT, 0);
		}
	}
}
