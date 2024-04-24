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
void SetupShadowPass();
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

    // Load lighting pass shader and set texture uniforms
    shaderLightingProgram = Shader::CreateShaderProgram("lighting.vert", "lighting.frag");
    glUseProgram(shaderLightingProgram);
    Shader::SetShaderUniformInt1i(&shaderLightingProgram, "tex", 0);
    Shader::SetShaderUniformInt1i(&shaderLightingProgram, "shadowMap", 1);

    // Load shadow pass shader
    shaderShadowProgram = Shader::CreateShaderProgram("shadow.vert", "shadow.frag");


    // Load models and set transform properties
    models.push_back(MeshPrimative::CreateCube());
    models.push_back(ModelLoader::LoadModel("Resources\\CasOC\\CASCAS.obj"));
    models.push_back(ModelLoader::LoadModel("Resources\\CUBE.obj"));
    models.push_back(ModelLoader::LoadModel("Resources\\Office.obj"));
    models.push_back(ModelLoader::LoadModel("Resources\\sphere.fbx"));
    models.push_back(MeshPrimative::CreateQuad());
    models.push_back(ModelLoader::LoadModel("Resources\\CUBE.obj"));

    models[0].position = glm::vec3(-0.5f, 1.0f, 0.0f);
    models[1].position = glm::vec3(2.0f, 0.0f, 0.0f);
    models[2].position = glm::vec3(6.5f, 1.0f, 0.0f);
    models[3].position = glm::vec3(-8.0f, 0.0f, 0.0f);
    models[4].position = glm::vec3(-2.5f, 1.0f, 0.0f);
    models[5].position = glm::vec3(-4.0f, 2.0f, 0.0f);
    models[5].rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    models[6].position = glm::vec3(0.0f, -0.01f, 0.0f);
    models[6].scale = glm::vec3(10.0f, 0.01f, 10.0f);
    models[6].uvTileFactor = 10.0f;


    SetupShadowPass();

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

    // Set directional light position
    //glm::vec3 directionalLightPosition = glm::vec3(glm::cos(glfwGetTime() * 0.5f) * 10, 5.0f, glm::sin(glfwGetTime() * 0.5f) * 10);
    glm::vec3 directionalLightPosition = glm::vec3(10.0f, 8.0f, 10.0f);
    Shader::SetShaderUniformVec3(&shaderLightingProgram, "lightPos", MathUtils::Vec3toFloat3(directionalLightPosition));

    // Render shadow pass
    RenderShadowPass(directionalLightPosition);

    // Render lighting pass
    RenderLightingPass();
}

void RenderShadowPass(glm::vec3 lightPosition)
{
    // Set up shadow pass
    glCullFace(GL_BACK);
    glViewport(0, 0, 4096 * 2, 4096 * 2);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);

    // Shader program to use for shadow pass
    glUseProgram(shaderShadowProgram);

    float near_plane = 1.0f, far_plane = 25.0f;
    glm::mat4 lightProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    Shader::SetShaderUniformgMatrix4fv(&shaderShadowProgram, "lightSpaceMatrix", lightSpaceMatrix);

    // Loop through models and draw them
    for (int i = 0; i < models.size(); i++)
    {
        ModelLoader::Model model = models[i];

        for (int j = 0; j < model.meshes.size(); j++)
        {
            ModelLoader::Mesh mesh = models[i].meshes[j];

            // Bind mesh VAO and EBO
            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

            // Calculate model matrix given model position, rotation and scale
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, model.position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0, 0, 1));
            modelMatrix = glm::scale(modelMatrix, model.scale);

            Shader::SetShaderUniformgMatrix4fv(&shaderShadowProgram, "model", modelMatrix);

            // Draw mesh
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderLightingPass()
{
    // Set up lighting pass
    glCullFace(GL_BACK);
    glViewport(0, 0, Engine::GetCurentScreenWidth(), Engine::GetCurentScreenHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Shader program to use for lighting pass
    glUseProgram(shaderLightingProgram);

    Camera::UpdateCamera(&shaderLightingProgram);

    Shader::SetShaderUniformgMatrix4fv(&shaderLightingProgram, "lightSpaceMatrix", lightSpaceMatrix);

    // Bind shadow map to texture slot 1 - only need to be bound once per pass
    Texture::SetActiveTexture(&shaderLightingProgram, &depthMap, 1);

    // Loop through models and draw them
    for (int i = 0; i < models.size(); i++)
    {
        ModelLoader::Model model = models[i];

        for (int j = 0; j < model.meshes.size(); j++)
        {
            ModelLoader::Mesh mesh = models[i].meshes[j];

            // Bind mesh VAO and EBO
            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

            // Calculate model matrix given model position, rotation and scale
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, model.position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0, 0, 1));
            modelMatrix = glm::scale(modelMatrix, model.scale);

            // Set camera forward vector
            float* forward = MathUtils::Vec3toFloat3(Camera::GetCameraForward());

            Shader::SetShaderUniformgMatrix4fv(&shaderLightingProgram, "model", modelMatrix);
            Shader::SetShaderUniformVec3(&shaderLightingProgram, "cameraForward", forward);
            Shader::SetShaderUniformInt1f(&shaderLightingProgram, "uvTileFactor", model.uvTileFactor);

            // Bind texture for each mesh
            if (mesh.textures.size() > 0)
            {
                Texture::SetActiveTexture(&shaderLightingProgram, &mesh.textures[0].id, 0);
            }

            // Draw mesh
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }
}

void SetupShadowPass() 
{
    // Generate new frame buffer and texture for shadow pass
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096 * 2, 4096 * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Set border color to white
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth map to frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    // Set draw buffer to none since we are only rendering depth
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Unbind frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int* Renderer::GetShaderProgram()
{
	return &shaderLightingProgram;
}