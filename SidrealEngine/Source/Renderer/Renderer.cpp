#include <filesystem>
#include <glad/glad.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include "Renderer.h"
#include "Shader.h"
#include "Utils/MathUtils.h"
#include "Model.h"
#include "Camera.h"
#include "MeshPrimative.h"
#include "Texture.h"
#include "Entity/Components/Transform.h"
#include "Entity/Components/RenderData.h"
#include <Window.h>

using namespace Components;

const unsigned int ShadowMapSize = 2048 * 2;

static void RenderSkybox();
static void InitializeShadowPass();
static void InitializeLightingPass();
static void UpdateLightProjectionViews();
static void RenderShadowPass(Model& model, Transform& transform, RenderData& renderData);
static void RenderLightingPass(Model& model, Transform& transform, RenderData& renderData);
static glm::mat4 CalculateLightSpaceMatrix(float nearPlane, float farPlane);

unsigned int shaderLightingProgram;
//unsigned int shaderLightingInstancedProgram;
unsigned int shaderShadowProgram;
//unsigned int shaderShadowInstancedProgram;
unsigned int shaderEquirectangularProgram;
unsigned int shaderDebugProgram;

unsigned int hdrTexture;
unsigned int matricesUBO;

unsigned int depthMaps;
unsigned int depthMapFBO;
std::vector<float> shadowCascadeLevels;
std::vector<glm::mat4> shadowLightSpaceMatricies;

glm::vec3 lightDirection;

Model skyboxModel;
RenderData skyboxRenderData;

void Renderer::Initialize()
{
    Camera::Initialize();
	SetLightDirection(MathUtils::Vec3toFloat3(glm::vec3(0.5f, -0.5f, -1.0f)));
    // Load shaders for each pass and set uniforms
    LoadShaders(false);

    // Create skybox model and HDR texture
    MeshPrimative::CreateCube(skyboxModel, skyboxRenderData);
    hdrTexture = Texture::LoadTextureHDR("Resources\\kloppenheim_06_puresky_4k.hdr");
    
    // Initialize shadow pass for rendering
    InitializeShadowPass();

	InitializeLightingPass();
    
    // Enable OpenGL features we want to use
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
}

void Renderer::SetupShadowPass()
{
    glCullFace(GL_FRONT);
    glViewport(0, 0, ShadowMapSize, ShadowMapSize);

	// Bind the matrices UBO to the shader program
    UpdateLightProjectionViews();
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, shadowLightSpaceMatricies.size() * sizeof(glm::mat4), shadowLightSpaceMatricies.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the depth map framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Shader program to use for shadow pass
    glUseProgram(shaderShadowProgram);
}

void Renderer::SetupLightingPass()
{
    // Set up lighting pass
    glViewport(0, 0, Window::GetCurentScreenWidth(), Window::GetCurentScreenHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render skybox seperate and behind from everything else
    RenderSkybox();

    // Set up lighting pass
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Shader program to use for lighting pass
    glUseProgram(shaderLightingProgram);

    Camera::UpdateCamera(&shaderLightingProgram);
    Shader::SetUniform3f(&shaderLightingProgram, "cameraForward", MathUtils::Vec3toFloat3(Camera::GetCameraForward()));
    Shader::SetUniform3f(&shaderLightingProgram, "lightDirection", MathUtils::Vec3toFloat3(glm::normalize(lightDirection)));

    // Bind shadow map to texture slot 1 - only need to be bound once per pass
    Texture::SetActiveAndBindTexture(depthMaps, 1);
}

void Renderer::RenderModel(Model& model, Components::Transform& transform, Components::RenderData renderData, PassType passType)
{
    if (passType == PassType::Shadow)
    {
        RenderShadowPass(model, transform, renderData);
    }
    else if (passType == PassType::Lighting)
    {
        RenderLightingPass(model, transform, renderData);
    }
}

// Summary: Load shaders for shadow and lighting passes
// Parameters: bool reload - if true, reload existing shaders
void Renderer::LoadShaders(bool reload)
{
    // If hot reloading shaders, destroy previous ones before loading new ones
    if (reload)
    {
        glUseProgram(0);
        glDeleteProgram(shaderLightingProgram);
        //glDeleteProgram(shaderLightingInstancedProgram);
        glDeleteProgram(shaderShadowProgram);
        //glDeleteProgram(shaderShadowInstancedProgram);
        glDeleteProgram(shaderEquirectangularProgram);
        glDeleteProgram(shaderDebugProgram);
        Shader::ClearUniformCache();
        std::cout << "Hot Reloading Shaders...\n";
    }
    else
    {
        auto shaderPath = (std::filesystem::current_path() / "Resources" / "Shaders").string();
        std::cout << "Loading Shaders from: " << shaderPath << "...\n";
	}

    shaderLightingProgram = Shader::CreateShaderProgram("Resources\\Shaders\\lighting.vert", "Resources\\Shaders\\lighting.frag");
    glUseProgram(shaderLightingProgram);
    Shader::SetUniform1i(&shaderLightingProgram, "colorTexture", 0);
    Shader::SetUniform1i(&shaderLightingProgram, "shadowMap", 1);
    Shader::SetUniform1i(&shaderLightingProgram, "cascadeCount", static_cast<int>(shadowCascadeLevels.size() + 1));
    Shader::SetUniform1fv(&shaderLightingProgram, "cascadePlaneDistances", shadowCascadeLevels.data(), static_cast<int>(shadowCascadeLevels.size()));

    shaderShadowProgram = Shader::CreateShaderProgram("Resources\\Shaders\\shadow.vert", "Resources\\Shaders\\shadow.frag", "Resources\\Shaders\\shadow.geom");
    glUseProgram(shaderShadowProgram);

    shaderEquirectangularProgram = Shader::CreateShaderProgram("Resources\\Shaders\\equirectangular.vert", "Resources\\Shaders\\equirectangular.frag");
    glUseProgram(shaderEquirectangularProgram);
    Shader::SetUniform1i(&shaderEquirectangularProgram, "equirectangularMap", 0);

    shaderDebugProgram = Shader::CreateShaderProgram("Resources\\Shaders\\debug.vert", "Resources\\Shaders\\debug.frag");
}

void Renderer::SetLightDirection(float* newDirection)
{
    lightDirection = glm::vec3(newDirection[0], newDirection[1], newDirection[2]);
}

float* Renderer::GetLightDirection()
{
    return MathUtils::Vec3toFloat3(lightDirection);
}


static void InitializeShadowPass()
{
	const unsigned int cascadeCount = 3;
	const float far = Camera::GetFarPlane();
	const float near = Camera::GetNearPlane();
	const float lambda = 0.5f;

    //            ---- Shadow Cascade Range ----
	//   nearplane | 1st cascade | 2nd cascade | farplane
	shadowCascadeLevels = std::vector<float>(cascadeCount - 1);
    for (int i = 1; i < cascadeCount; ++i)
    {
        float p = i / static_cast<float>(3);
        float logSplit = near * std::pow(far / near, p);
        float uniformSplit = near + (far - near) * p;
        shadowCascadeLevels[i - 1] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
    }
    shadowLightSpaceMatricies = std::vector<glm::mat4>(cascadeCount);

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMaps);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, ShadowMapSize, ShadowMapSize,
        int(shadowCascadeLevels.size() + 1), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
        throw 0;
    }

    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 16, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static void InitializeLightingPass()
{
    glUseProgram(shaderLightingProgram);
    Camera::UpdateCamera(&shaderLightingProgram);
    Shader::SetUniform3f(&shaderLightingProgram, "cameraForward", MathUtils::Vec3toFloat3(Camera::GetCameraForward()));
    Shader::SetUniform3f(&shaderLightingProgram, "lightDirection", MathUtils::Vec3toFloat3(glm::normalize(lightDirection)));
    Shader::SetUniform1i(&shaderLightingProgram, "cascadeCount", static_cast<int>(shadowCascadeLevels.size() + 1));
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        std::string uniformName = "cascadePlaneDistances[" + std::to_string(i) + "]";
        Shader::SetUniform1f(&shaderLightingProgram, uniformName.c_str(), shadowCascadeLevels[i]);
    }
}

static void renderQuad(int i);
void Renderer::ShowDepthMapDebug()
{
    // Shader program to use for lighting pass
    glUseProgram(shaderDebugProgram);
    Shader::SetUniform1i(&shaderDebugProgram, "depthMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
    for (int i = 0; i < shadowCascadeLevels.size() + 1; i++)
    {
        Shader::SetUniform1i(&shaderDebugProgram, "layer", i);
        renderQuad(i);
	}
}


static void RenderSkybox()
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
    Mesh& mesh = skyboxModel.meshes[0];
    glBindVertexArray(skyboxRenderData.renderMeshData[0].vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxRenderData.renderMeshData[0].ebo);

    // Bind HDR texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    // Draw the skybox model
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);

    // Unbind vertex array and set depth func back to default
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

static void RenderShadowPass(Model& model, Components::Transform& transform, RenderData& renderData)
{
    for (int j = 0; j < model.meshes.size(); j++)
    {
        Mesh& mesh = model.meshes[j];

        // Bind mesh VAO and EBO
        glBindVertexArray(renderData.renderMeshData[j].vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData.renderMeshData[j].ebo);

        Shader::SetMatrix4f(&shaderShadowProgram, "model", Components::GetModelMatrix(transform));

        GLsizei indices = static_cast<GLsizei>(mesh.indices.size());

        // Draw mesh
        glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
    }
}

static void RenderLightingPass(Model& model, Components::Transform& transform, RenderData& renderData)
{
    for (int j = 0; j < model.meshes.size(); j++)
    {
        Mesh& mesh = model.meshes[j];

        // Bind mesh VAO and EBO
        glBindVertexArray(renderData.renderMeshData[j].vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData.renderMeshData[j].ebo);

        Shader::SetMatrix4f(&shaderLightingProgram, "model", Components::GetModelMatrix(transform));

        // Bind texture for each mesh
        if (renderData.renderMeshData[j].texture >= 0)
        {
            Shader::SetUniform1f(&shaderLightingProgram, "uvTileFactor", model.uvTileFactor);
            //if (model.name == "quad.obj")   Texture::SetActiveAndBindTexture(depthMaps, 0);
            Texture::SetActiveAndBindTexture(renderData.renderMeshData[j].texture, 0);
        }

        // Draw mesh
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
    }
}

static void UpdateLightProjectionViews()
{
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            shadowLightSpaceMatricies[i] = CalculateLightSpaceMatrix(Camera::GetNearPlane(), shadowCascadeLevels[i]);
        }
        else if (i < shadowCascadeLevels.size())
        {
            shadowLightSpaceMatricies[i] = CalculateLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]);
        }
        else
        {
            shadowLightSpaceMatricies[i] = CalculateLightSpaceMatrix(shadowCascadeLevels[i - 1], Camera::GetFarPlane());
        }
    }
}

static glm::mat4 CalculateLightSpaceMatrix(float nearPlane, float farPlane)
{
    const auto cameraProjection = glm::perspective(glm::radians(Camera::GetFOV()), 
        (float)Window::GetCurentScreenWidth() / (float)Window::GetCurentScreenHeight(), nearPlane, farPlane);

	glm::mat4 inverseViewProjection = glm::inverse(cameraProjection * Camera::GetViewMatrix());

	// Get the frustum corners in world space
    std::vector<glm::vec4> frustumCorners;
    frustumCorners.reserve(8);
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 2; ++z) {
                glm::vec4 corner = glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f,
                    2.0f * z - 1.0f,
                    1.0f
                );
                glm::vec4 worldSpace = inverseViewProjection * corner;
                worldSpace /= worldSpace.w;
                frustumCorners.push_back(worldSpace);
            }
        }
    }

	// Transform the frustum corners to world space and find the center of the frustum
	glm::vec3 frustumCenter = glm::vec3(0.0f);
    for (int i = 0; i < frustumCorners.size(); i++)
    {
		frustumCenter += glm::vec3(frustumCorners[i]);
    }
	frustumCenter /= frustumCorners.size();

	// Calculate the light view matrix
    glm::mat4 lightView = glm::lookAt(
        frustumCenter - glm::normalize(lightDirection),
        frustumCenter,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Transform frustum corners to light space
    std::vector<glm::vec3> lightSpaceCorners;
    lightSpaceCorners.reserve(frustumCorners.size());
    for (const auto& c : frustumCorners)
        lightSpaceCorners.push_back(glm::vec3(lightView * c));

    // Calculate min and max bounds in light space (for ortho projection)
    glm::vec3 minBounds = lightSpaceCorners[0];
    glm::vec3 maxBounds = lightSpaceCorners[0];
    for (const auto& c : lightSpaceCorners)
    {
        minBounds = glm::min(minBounds, c);
        maxBounds = glm::max(maxBounds, c);
    }

    // Add padding to reduce edge artifacts
    const float padding = 10.0f; // Adjust based on your scene scale
    minBounds.x -= padding;
    minBounds.y -= padding;
    maxBounds.x += padding;
    maxBounds.y += padding;

	// Adjust bounds to maintain aspect ratio of the shadow map
    float width = maxBounds.x - minBounds.x;
    float height = maxBounds.y - minBounds.y;
    float aspect = 1.0f; 

    if (width > height * aspect)
    {
        float newHeight = width / aspect;
        float diff = newHeight - height;
        minBounds.y -= diff * 0.5f;
        maxBounds.y += diff * 0.5f;
    }
    else
    {
        float newWidth = height * aspect;
        float diff = newWidth - width;
        minBounds.x -= diff * 0.5f;
        maxBounds.x += diff * 0.5f;
    }


    // Near/far planes in light space, with a bit of padding
    float nearPlaneLight = -maxBounds.z - padding;  // note: light looks down -Z
    float farPlaneLight = -minBounds.z + padding;

    // Create orthographic projection bounds
    float left = minBounds.x;
    float right = maxBounds.x;
    float bottom = minBounds.y;
    float top = maxBounds.y;


    glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, nearPlaneLight, farPlaneLight);

    // Calculate texel size in light space
    float texelSize = (right - left) / static_cast<float>(ShadowMapSize);

    // Snap the light view matrix's position to texel-sized increments
    lightView[3][0] -= glm::mod(lightView[3][0], texelSize);
    lightView[3][1] -= glm::mod(lightView[3][1], texelSize);

    // Return the final light-space transformation matrix
    return lightProjection * lightView;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
static void renderQuad(int i)
{
    if (quadVAO == 0)
    {
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast <GLsizeiptr>(sizeof(float) * 4.0f * 5.0f), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    float quadVertices[] = {
        // positions          // texture Coords
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  0.0f, 0.0f,   0.0f, 1.0f, // top left
         0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom right
         0.0f,  0.0f, 0.0f,   1.0f, 1.0f, // top right
    };

    // Scale quad vertices to fit the screen aspect ratio and push it to the bottom left corner
    int height = Window::GetCurentScreenHeight();
    int width = Window::GetCurentScreenWidth();
    float aspect = float(width) / float(height);
    if (aspect > 1.0f)
    {
        // Scale X and shift so left edge stays at -1
        float xScale = 1.0f / aspect;
        for (int i = 0; i < 4; ++i)
        {
            float& x = quadVertices[i * 5 + 0];
            x = -1.0f + (x + 1.0f) * xScale; // move from [-1,0] ¨ scaled range starting at -1
        }
    }
    else
    {
        // Scale Y and shift so bottom edge stays at -1
        float yScale = aspect;
        for (int i = 0; i < 4; ++i)
        {
            float& y = quadVertices[i * 5 + 1];
            y = -1.0f + (y + 1.0f) * yScale; // move from [-1,0] ¨ scaled range starting at -1
        }
    }

    // Push to side depending on i
    for (int j = 0; j < 4; j++)
    {
        quadVertices[0 + j * 5] += i * 0.5f;
    }

    float scale = 0.5f; // scale down to 50%

    for (int i = 0; i < 4; ++i)
    {
        float& x = quadVertices[i * 5 + 0];
        float& y = quadVertices[i * 5 + 1];

        x = (x + 1.0f) * scale - 1.0f;
        y = (y + 1.0f) * scale - 1.0f;
    }

    // Update buffer data
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVertices), quadVertices);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}