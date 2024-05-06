#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "Renderer.h"
#include "MathUtils.h"
#include "Camera.h"
#include "MeshPrimative.h"
#include "ModelLoader.h"
#include "Shader.h"
#include "Texture.h"
#include "Engine.h"
#include "Timer.h"
#include "AssetLoader.h"

const unsigned int ShadowMapSize = 2048 * 8;

void RenderShadowPass(glm::vec3 lightPosition);
void RenderLightingPass();
void SetupShadowPass();
void RenderSkybox();

unsigned int shaderShadowProgram;
unsigned int shaderLightingProgram;
unsigned int shaderEquirectangularProgram;
unsigned int* textures;

unsigned int hdrTexture;
unsigned int captureFBO;
unsigned int textureColorbuffer;

double lastTime;
double counter;
unsigned int currentTexture;
unsigned int depthMapFBO;
unsigned int depthMap;
glm::mat4 lightSpaceMatrix;

ModelLoader::Model skyboxModel;

std::vector<ModelLoader::Model> models;

void Renderer::Initialize()
{
    Camera::Initialize();

    // Load shaders for each pass and set uniforms
    LoadShaders(false);

    // Create skybox model and HDR texture
    skyboxModel = MeshPrimative::CreateCube();
    hdrTexture = Texture::CreateTextureHDR("Resources\\kloppenheim_06_puresky_4k.hdr");
    
	models = AssetLoader::LoadAllAssets();
    
    //// spawn 50 random CasOC models
    //for (int i = 0; i < 20; i++)
    //{
    //    for (int j = 0; j < 20; j++)
    //    {
    //        ModelLoader::Model model = ModelLoader::LoadModel("Resources\\CasOC\\CASCAS.obj");
    //        model.position = glm::vec3(1 * i, 0.0f, 1 * j);
    //        models.push_back(model);
    //    }
	//}

    // Calculate the rough number of draw calls.
    int drawCalls = 0;
    for (int i = 0; i < models.size(); i++)
    {
        for(int j = 0; j < models[i].meshes.size(); j++)
        {
            drawCalls++;
        }
    }
    drawCalls *= 2; // Shadow pass 
    drawCalls += 1; // Skybox
    std::cout << "Draw Calls: " << drawCalls << std::endl;

    // Initialize shadow pass for rendering
    SetupShadowPass();

    // Enable OpenGL features we want to use
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
}

void Renderer::Render()
{
    Timer::Start("Full Render");

    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;

    glClearColor(0.0f, 1.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set directional light position
    //glm::vec3 directionalLightPosition = glm::vec3(glm::cos(glfwGetTime() * 0.5f) * 10, 8.0f, glm::sin(glfwGetTime() * 0.5f) * 10);
    glm::vec3 directionalLightPosition = glm::vec3(10.0f, 8.0f, 10.0f);
    directionalLightPosition += Camera::GetCameraPosition();
    Shader::SetUniform3f(&shaderLightingProgram, "lightPos", MathUtils::Vec3toFloat3(directionalLightPosition));

    // Render shadow pass
    Timer::Start("Shadow Pass");
    RenderShadowPass(directionalLightPosition);
    Timer::Stop("Shadow Pass");


    // Render lighting pass
    Timer::Start("Lighting Pass");
    RenderLightingPass();
    Timer::Stop("Lighting Pass");

    Timer::Stop("Full Render");
}

void RenderShadowPass(glm::vec3 lightPosition)
{
    // Set up shadow pass
    glCullFace(GL_BACK);
    glViewport(0, 0, ShadowMapSize, ShadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);

    // Shader program to use for shadow pass
    glUseProgram(shaderShadowProgram);

    float near_plane = 1.0f, far_plane = 40.0f;
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + glm::vec3(-0.5f, -1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    Shader::SetMatrix4f(&shaderShadowProgram, "lightSpaceMatrix", lightSpaceMatrix);

    // Loop through models and draw them
    for (int i = 0; i < models.size(); i++)
    {
        ModelLoader::Model& model = models[i];

        // Calculate model matrix given model position, rotation and scale
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, model.position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0, 0, 1));
        modelMatrix = glm::scale(modelMatrix, model.scale);
        Shader::SetMatrix4f(&shaderShadowProgram, "model", modelMatrix);

        for (int j = 0; j < model.meshes.size(); j++)
        {
            ModelLoader::Mesh& mesh = models[i].meshes[j];

            // Bind mesh VAO and EBO
            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

            // Draw mesh
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderLightingPass()
{
    // Set up lighting pass
    glViewport(0, 0, Engine::GetCurentScreenWidth(), Engine::GetCurentScreenHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render skybox seperate and behind from everything else
    RenderSkybox();

    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    glClear( GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Shader program to use for lighting pass
    glUseProgram(shaderLightingProgram);

    Camera::UpdateCamera(&shaderLightingProgram);

    Shader::SetMatrix4f(&shaderLightingProgram, "lightSpaceMatrix", lightSpaceMatrix);
    // Set camera forward vector
    float* forward = MathUtils::Vec3toFloat3(Camera::GetCameraForward());
    Shader::SetUniform3f(&shaderLightingProgram, "cameraForward", forward);

    // Bind shadow map to texture slot 1 - only need to be bound once per pass
    Texture::SetActiveTexture(&shaderLightingProgram, &depthMap, 1);
    // Loop through models and draw them
    for (int i = 0; i < models.size(); i++)
    {
        ModelLoader::Model& model = models[i];

        // Calculate model matrix given model position, rotation and scale
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, model.position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0, 0, 1));
        modelMatrix = glm::scale(modelMatrix, model.scale);
        Shader::SetMatrix4f(&shaderLightingProgram, "model", modelMatrix);

        for (int j = 0; j < model.meshes.size(); j++)
        {
            ModelLoader::Mesh& mesh = models[i].meshes[j];

            // Bind mesh VAO and EBO
            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

            Shader::SetUniform1f(&shaderLightingProgram, "uvTileFactor", model.uvTileFactor);

            // Bind texture for each mesh
            if (mesh.textures.size() > 0)
            {
                Texture::SetActiveTexture(&shaderLightingProgram, &mesh.textures[0].id, 0);
            }

            if (i == 5) {
                Texture::SetActiveTexture(&shaderLightingProgram, &depthMap, 0);
            }

            // Draw mesh
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }

}

void SetupShadowPass() 
{
    // Generate new frame buffer and texture for shadow pass
    glCullFace(GL_BACK);
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapSize, ShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

// Summary: Load shaders for shadow and lighting passes
// Parameters: bool reload - if true, reload existing shaders
void Renderer::LoadShaders(bool reload)
{
    // Create shader for main lighting pass and setup texture targets
    unsigned int lightProgram = Shader::CreateShaderProgram("lighting.vert", "lighting.frag");
    glUseProgram(lightProgram);
    Shader::SetInt1i(&lightProgram, "tex", 0);
    Shader::SetInt1i(&lightProgram, "shadowMap", 1);

    // Create shadr for shadow mapping
    unsigned int shadowProgram = Shader::CreateShaderProgram("shadow.vert", "shadow.frag");

    // Create shader for skybox and setup texture target
    unsigned int equirectangularProgram = Shader::CreateShaderProgram("equirectangular.vert", "equirectangular.frag");
    glUseProgram(equirectangularProgram);
    Shader::SetInt1i(&equirectangularProgram, "equirectangularMap", 0);

    // If hot reloading shaders, destroy previous ones before loading new ones
    if (reload) {
        glDeleteProgram(shaderLightingProgram);
        glDeleteProgram(shaderShadowProgram);
        glDeleteProgram(shaderEquirectangularProgram);
    }

    // Assign newly loaded shaders to be used
    shaderLightingProgram = lightProgram;
    shaderShadowProgram = shadowProgram;
    shaderEquirectangularProgram = equirectangularProgram;
}

void RenderSkybox()
{
    // Only render the inside
    glCullFace(GL_FRONT);

    // Draw skybox behind everything else
    glDepthFunc(GL_LEQUAL);
    glUseProgram(shaderEquirectangularProgram);

    // Leaving the left column and bottom row as zero removes any transform from the camera.
    // This keeps it directly placed around the camera's view.
    glm::mat4 view = glm::mat4(glm::mat3(Camera::GetViewMatrix()));
    Shader::SetMatrix4f(&shaderEquirectangularProgram, "view", view);
    Shader::SetMatrix4f(&shaderEquirectangularProgram, "projection", Camera::GetProjectionMatrix());
    
    // Render the skybox cube mesh
    ModelLoader::Mesh& mesh = skyboxModel.meshes[0];
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind vertex array and set depth func back to default
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

}