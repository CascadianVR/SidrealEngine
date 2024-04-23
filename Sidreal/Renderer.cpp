#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "MathUtils.h"
#include "MeshPrimative.h"
#include "ModelLoader.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "Engine.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>

void RenderScene(unsigned int* shaderShadowProgram);
void RenderShadowPass(glm::vec3 lightPosition);
void RenderLightingPass();
char* LoadShader(const char* path);

unsigned int shaderShadowProgram;
unsigned int shaderLightingProgram;
unsigned int* textures;

double lastTime;
double counter;
unsigned int currentTexture;
unsigned int depthMapFBO;
unsigned int depthMap;
glm::mat4 lightSpaceMatrix;

std::vector<ModelLoader::Model> models;

void Renderer::Initialize()
{
    Camera::Initialize();

    shaderLightingProgram = Shader::CreateShaderProgram("lighting.vert", "lighting.frag");
    Shader::SetShaderUniformInt1i(&shaderLightingProgram, "tex", 0);
    shaderShadowProgram = Shader::CreateShaderProgram("shadow.vert", "shadow.frag");

    models.push_back(MeshPrimative::CreateCube());
    models.push_back(ModelLoader::LoadModel("Resources\\CasOC\\CASCAS.obj"));
    models.push_back(ModelLoader::LoadModel("Resources\\CUBE.obj"));
    models.push_back(ModelLoader::LoadModel("Resources\\Office.obj"));
    models.push_back(ModelLoader::LoadModel("Resources\\sphere.obj"));
    models.push_back(MeshPrimative::CreateQuad());
    models.push_back(ModelLoader::LoadModel("Resources\\CUBE.obj"));

    models[0].position = glm::vec3(-0.5f, 1.0f, 0.0f);

    models[1].position = glm::vec3(2.0f, 0.0f, 0.0f);

    models[2].position = glm::vec3(4.0f, 1.0f, 0.0f);

    models[3].position = glm::vec3(-8.0f, 0.0f, 0.0f);

    models[4].position = glm::vec3(-2.5f, 1.0f, 0.0f);
    
    models[5].position = glm::vec3(-4.0f, 2.0f, 0.0f);
    models[5].rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    
    models[6].position = glm::vec3(0.0f, -0.01f, 0.0f);
    models[6].scale = glm::vec3(10.0f, 0.01f, 10.0f);
    models[6].uvTileFactor = 10.0f;

    std::cout << models[0].meshes[0].textures.size() << std::endl;

    // Shadows
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    // Set directional light position
    glm::vec3 lightPosz = glm::vec3(glm::cos(glfwGetTime()) * 6, 3.0f, glm::sin(glfwGetTime()) * 6);
    Shader::SetShaderUniformVec3(&shaderLightingProgram, "lightPos", MathUtils::Vec3toFloat3(lightPosz));

    RenderShadowPass(lightPosz);

    RenderLightingPass();
}

void RenderShadowPass(glm::vec3 lightPosition)
{
    glCullFace(GL_BACK);
    glViewport(0, 0, 4096, 4096);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera::UpdateCamera(&shaderShadowProgram);
    float near_plane = 3.0f, far_plane = 20.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
    Shader::SetShaderUniformgMatrix4fv(&shaderShadowProgram, "lightSpaceMatrix", lightSpaceMatrix);

    RenderScene(&shaderShadowProgram);
}

void RenderLightingPass()
{
    glCullFace(GL_BACK);
    glViewport(0, 0, Engine::GetCurentScreenWidth(), Engine::GetCurentScreenHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 1.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera::UpdateCamera(&shaderLightingProgram);

    RenderScene(&shaderLightingProgram);
}

void RenderScene(unsigned int* shaderProgram)
{
    // Loop through models and draw them
    for (int i = 0; i < models.size(); i++)
    {
        ModelLoader::Model model = models[i];
        for (int j = 0; j < model.meshes.size(); j++)
        {
            ModelLoader::Mesh mesh = models[i].meshes[j];

            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, model.position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0, 0, 1));
            modelMatrix = glm::scale(modelMatrix, model.scale);
            Shader::SetShaderUniformgMatrix4fv(shaderProgram, "model", modelMatrix);

            Shader::SetShaderUniformgMatrix4fv(shaderProgram, "lightSpaceMatrix", lightSpaceMatrix);

            glBindTexture(GL_TEXTURE_2D, depthMap);
            Shader::SetShaderUniformInt1i(&shaderLightingProgram, "shadowMap", 1);

            float* forward = MathUtils::Vec3toFloat3(Camera::GetCameraForward());
            Shader::SetShaderUniformVec3(shaderProgram, "cameraForward", forward);

            Shader::SetShaderUniformInt1f(shaderProgram, "uvTileFactor", model.uvTileFactor);

            Texture::SetActiveTexture(shaderProgram, &depthMap, 1);

            glUseProgram(*shaderProgram);
            if (mesh.textures.size() > 0)
            {
                Texture::SetActiveTexture(shaderProgram, &mesh.textures[0].id, 0);
            }

            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }
}

unsigned int* Renderer::GetShaderProgram()
{
	return &shaderLightingProgram;
}