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

const unsigned int ShadowMapSize = 2048 * 4;

void RenderShadowPass(glm::vec3 lightPosition);
void RenderLightingPass();
void SetupShadowPass();
void RenderCube();


unsigned int captureFBO;

unsigned int shaderShadowProgram;
unsigned int shaderLightingProgram;
unsigned int shaderEquirectangularProgram;
unsigned int* textures;
unsigned int hdrTexture;

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

    // Load shaders for each pass and set uniforms
    LoadShaders(false);

    hdrTexture = Texture::CreateTextureHDR("Resources\\cubemap.png");
    glGenFramebuffers(1, &captureFBO);

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

    // spawn 50 random CasOC models
    
    //for (int i = 0; i < 20; i++)
    //{
    //    for (int j = 0; j < 20; j++)
    //    {
    //        ModelLoader::Model model = ModelLoader::LoadModel("Resources\\CasOC\\CASCAS.obj");
    //        model.position = glm::vec3(1 * i, 0.0f, 1 * j);
    //        models.push_back(model);
    //    }
	//}

    int drawCalls = 0;
    for (int i = 0; i < models.size(); i++)
    {
        for(int j = 0; j < models[i].meshes.size(); j++)
        {
            drawCalls++;
        }
    }

    std::cout << "Draw Calls: " << drawCalls << std::endl;


    SetupShadowPass();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Renderer::Render()
{
    Timer::Start("Full Render");

    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;
    counter += delta;

    glClearColor(0.0f, 1.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set directional light position
    glm::vec3 directionalLightPosition = glm::vec3(glm::cos(glfwGetTime() * 0.5f) * 10, 8.0f, glm::sin(glfwGetTime() * 0.5f) * 10);
    //glm::vec3 directionalLightPosition = glm::vec3(10.0f, 8.0f, 10.0f);
    Shader::SetUniform3f(&shaderLightingProgram, "lightPos", MathUtils::Vec3toFloat3(directionalLightPosition));

    // Render shadow pass
    Timer::Start("Shadow Pass");
    RenderShadowPass(directionalLightPosition);
    Timer::Stop("Shadow Pass");

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };


    //// convert HDR equirectangular environment map to cubemap equivalent
    //glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    //glUseProgram(shaderEquirectangularProgram);
    //Shader::SetInt1i(&shaderEquirectangularProgram, "equirectangularMap", 0);
    //Shader::SetMatrix4f(&shaderEquirectangularProgram, "projection", captureProjection);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, hdrTexture);
    //RenderCube();
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    float near_plane = 1.0f, far_plane = 25.0f;
    glm::mat4 lightProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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
    glCullFace(GL_BACK);
    glViewport(0, 0, Engine::GetCurentScreenWidth(), Engine::GetCurentScreenHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

// Summary: Load shaders for shadow and lighting passes
// Parameters: bool reload - if true, reload existing shaders
void Renderer::LoadShaders(bool reload)
{
    unsigned int lightProgram = Shader::CreateShaderProgram("lighting.vert", "lighting.frag");
    glUseProgram(lightProgram);
    Shader::SetInt1i(&lightProgram, "tex", 0);
    Shader::SetInt1i(&lightProgram, "shadowMap", 1);

    unsigned int shadowProgram = Shader::CreateShaderProgram("shadow.vert", "shadow.frag");
    unsigned int equirectangularProgram = Shader::CreateShaderProgram("equirectangular.vert", "equirectangular.frag");

    if (reload) {
        glDeleteProgram(shaderLightingProgram);
        glDeleteProgram(shaderShadowProgram);
    }

    shaderLightingProgram = lightProgram;
    shaderShadowProgram = shadowProgram;
    shaderEquirectangularProgram = shadowProgram;
}

void RenderCube() {

    float cubeVertices[] = {
            // positions          // texture coords
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

            -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

             0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

             0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}