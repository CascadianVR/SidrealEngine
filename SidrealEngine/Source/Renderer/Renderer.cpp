#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "Renderer.h"
#include "Shader.h"
#include "../MathUtils.h"
#include "Model.h"
#include "../Camera.h"
#include "../MeshPrimative.h"
#include "../Texture.h"
#include "../Engine.h"
#include "../Entity/Components/Transform.h"

const unsigned int ShadowMapSize = 2048 * 2;

void RenderSkybox();
void InitializeShadowPass();
void UpdateLightProjectionViews();
static void RenderShadowPass(Model& model, EntityTransform::Transform& transform);
static void RenderLightingPass(Model& model, EntityTransform::Transform& transform);
glm::mat4 CalculateLightSpaceMatrix(float nearPlane, float farPlane);

unsigned int shaderLightingProgram;
unsigned int shaderLightingInstancedProgram;
unsigned int shaderShadowProgram;
unsigned int shaderShadowInstancedProgram;
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

void Renderer::Initialize()
{
    Camera::Initialize();
	SetLightDirection(MathUtils::Vec3toFloat3(glm::vec3(0.5f, -0.5f, -1.0f)));
    // Load shaders for each pass and set uniforms
    LoadShaders(false);

    // Create skybox model and HDR texture
    skyboxModel = MeshPrimative::CreateCube();
    hdrTexture = Texture::CreateTextureHDR("Resources\\kloppenheim_06_puresky_4k.hdr");
    
	Scene::SceneData& sceneData = *Scene::GetActiveScene();

    std::cout << "Total models loaded: " << sceneData.models.size() << std::endl;
    
    // Calculate the rough number of draw calls.
    int drawCalls = 0;
    for (int i = 0; i < sceneData.models.size(); i++)
    {
        Model* model = sceneData.models[i];
        for(int j = 0; j < model->meshes.size(); j++)
        {
            drawCalls++;
        }
    }
    drawCalls *= 2; // Shadow pass 
    drawCalls += 1; // Skybox
    std::cout << "Draw Calls: " << drawCalls << std::endl;
    
    // Initialize shadow pass for rendering
    InitializeShadowPass();

    glUseProgram(shaderLightingProgram);

    Camera::UpdateCamera(&shaderLightingProgram);
    Shader::SetMatrix4f(&shaderLightingProgram, "lightSpaceMatrix", shadowLightSpaceMatricies[0]);
    Shader::SetUniform3f(&shaderLightingProgram, "cameraForward", MathUtils::Vec3toFloat3(Camera::GetCameraForward()));
    Shader::SetUniform3f(&shaderLightingProgram, "lightDirection", MathUtils::Vec3toFloat3(lightDirection));
    Shader::SetUniform1f(&shaderLightingProgram, "nearPlane", Camera::GetNearPlane());
    Shader::SetUniform1f(&shaderLightingProgram, "farPlane", Camera::GetFarPlane());
    Shader::SetInt1i(&shaderLightingProgram, "cascadeCount", shadowCascadeLevels.size()+1);
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        std::string uniformName = "cascadePlaneDistances[" + std::to_string(i) + "]";
        Shader::SetUniform1f(&shaderLightingProgram, uniformName.c_str(), shadowCascadeLevels[i]);
    }
    
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
    glViewport(0, 0, Engine::GetCurentScreenWidth(), Engine::GetCurentScreenHeight());
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
    Shader::SetMatrix4f(&shaderLightingProgram, "lightSpaceMatrix", shadowLightSpaceMatricies[0]);
    Shader::SetUniform3f(&shaderLightingProgram, "cameraForward", MathUtils::Vec3toFloat3(Camera::GetCameraForward()));
    Shader::SetUniform3f(&shaderLightingProgram, "lightDirection", MathUtils::Vec3toFloat3(lightDirection));

    // Bind shadow map to texture slot 1 - only need to be bound once per pass
    Texture::SetActiveAndBindTexture(depthMaps, 1);
}

void Renderer::RenderModel(Model& model, EntityTransform::Transform& transform, PassType passType)
{
    if (passType == PassType::Shadow)
    {
        RenderShadowPass(model, transform);
    }
    else if (passType == PassType::Lighting)
    {
        RenderLightingPass(model, transform);
    }
}

// Summary: Load shaders for shadow and lighting passes
// Parameters: bool reload - if true, reload existing shaders
void Renderer::LoadShaders(bool reload)
{
    // If hot reloading shaders, destroy previous ones before loading new ones
    if (reload)
    {
        glDeleteProgram(shaderLightingProgram);
        glDeleteProgram(shaderLightingInstancedProgram);
        glDeleteProgram(shaderShadowProgram);
        glDeleteProgram(shaderShadowInstancedProgram);
        glDeleteProgram(shaderEquirectangularProgram);
        glDeleteProgram(shaderDebugProgram);

        std::cout << "Hot Reloading Shaders...\n";
    }

    glFlush();

    shaderLightingProgram = Shader::CreateShaderProgram("Resources\\Shaders\\lighting.vert", "Resources\\Shaders\\lighting.frag");
    glUseProgram(shaderLightingProgram);
    Shader::SetInt1i(&shaderLightingProgram, "tex", 0);
    Shader::SetInt1i(&shaderLightingProgram, "shadowMap", 1);

    shaderShadowProgram = Shader::CreateShaderProgram("Resources\\Shaders\\shadow.vert", "Resources\\Shaders\\shadow.frag", "Resources\\Shaders\\shadow.geom");
    glUseProgram(shaderShadowProgram);
    Shader::SetMatrix4f(&shaderShadowProgram, "lightSpaceMatrix", 0);

    shaderEquirectangularProgram = Shader::CreateShaderProgram("Resources\\Shaders\\equirectangular.vert", "Resources\\Shaders\\equirectangular.frag");
    glUseProgram(shaderEquirectangularProgram);
    Shader::SetInt1i(&shaderEquirectangularProgram, "equirectangularMap", 0);

    shaderDebugProgram = Shader::CreateShaderProgram("Resources\\Shaders\\debug.vert", "Resources\\Shaders\\debug.frag");
}

void Renderer::SetLightDirection(float* newDirection)
{
    lightDirection = glm::normalize(glm::vec3(newDirection[0], newDirection[1], newDirection[2]));
}

float* Renderer::GetLightDirection()
{
    return MathUtils::Vec3toFloat3(lightDirection);
}

void InitializeShadowPass()
{
	const float near = Camera::GetNearPlane();
	const float far = Camera::GetFarPlane();

    //              ---- Shadow Cascade Range ----
	/// | nearplane | 1st cascade | 2nd cascade | farplane |
    shadowCascadeLevels = {
        near + (far - near) * 0.15f,  // end of 1st cascade (~10% of view distance)
        near + (far - near) * 0.25f   // end of 2nd cascade (~50% of view distance)
    };
    shadowLightSpaceMatricies = std::vector<glm::mat4>(shadowCascadeLevels.size() + 1);

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMaps);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, ShadowMapSize, ShadowMapSize,
        int(shadowCascadeLevels.size() + 1), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

void renderQuad(int i);
void Renderer::ShowDepthMapDebug()
{
    // Shader program to use for lighting pass
    glUseProgram(shaderDebugProgram);
    Shader::SetInt1i(&shaderDebugProgram, "depthMap", 0);
    Shader::SetUniform1f(&shaderDebugProgram, "near_plane", Camera::GetNearPlane());
    Shader::SetUniform1f(&shaderDebugProgram, "far_plane", Camera::GetFarPlane());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
    for (int i = 0; i < shadowCascadeLevels.size() + 1; i++)
    {
        Shader::SetInt1i(&shaderDebugProgram, "layer", i);
        renderQuad(i);
	}
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
    Mesh& mesh = skyboxModel.meshes[0];
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

    // Bind HDR texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    // Draw the skybox model
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind vertex array and set depth func back to default
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

static void RenderShadowPass(Model& model, EntityTransform::Transform& transform)
{
    for (int j = 0; j < model.meshes.size(); j++)
    {
        Mesh& mesh = model.meshes[j];

        // Bind mesh VAO and EBO
        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

        Shader::SetMatrix4f(&shaderShadowProgram, "model", EntityTransform::GetModelMatrix(transform));
        Shader::SetInt1i(&shaderShadowProgram, "cascadeIndex", 0);

        // Draw mesh
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

static void RenderLightingPass(Model& model, EntityTransform::Transform& transform)
{
    for (int j = 0; j < model.meshes.size(); j++)
    {
        Mesh& mesh = model.meshes[j];

        // Bind mesh VAO and EBO
        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

        Shader::SetMatrix4f(&shaderLightingProgram, "model", EntityTransform::GetModelMatrix(transform));

        // Bind texture for each mesh
        if (mesh.textures.size() > 0)
        {
            Shader::SetUniform1f(&shaderLightingProgram, "uvTileFactor", model.uvTileFactor);
            //if (model.name == "quad.obj")   Texture::SetActiveAndBindTexture(depthMaps, 0);
            Texture::SetActiveAndBindTexture(mesh.textures[0].id, 0);
        }

        // Draw mesh
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

void UpdateLightProjectionViews()
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
    //for (int i = 0; i < shadowCascadeLevels.size() + 1; i++) 
    //{
    //    shadowLightSpaceMatricies[i] = CalculateLightSpaceMatrix(lightDirection, i);
    //}
}

glm::mat4 CalculateLightSpaceMatrix(float nearPlane, float farPlane) 
{
    //float nearPlane = (index == 0) ? Camera::GetNearPlane() : shadowCascadeLevels[index - 1];
    //float farPlane = (index < shadowCascadeLevels.size()) ? shadowCascadeLevels[index] : Camera::GetFarPlane();
    //float overlap = 0.5f * (farPlane - nearPlane); // ~1% overlap
	//nearPlane -= overlap;
	//farPlane += overlap;

    const auto cameraProjection = glm::perspective(glm::radians(Camera::GetFOV()), 
        (float)Engine::GetCurentScreenWidth() / (float)Engine::GetCurentScreenHeight(), nearPlane, farPlane);

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
        frustumCenter - lightDirection,
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

    // Near/far planes in light space, with a bit of padding
    float nearPlaneLight = -maxBounds.z - padding;  // note: light looks down -Z
    float farPlaneLight = -minBounds.z + padding;

    // Create orthographic projection bounds
    float left = minBounds.x;
    float right = maxBounds.x;
    float bottom = minBounds.y;
    float top = maxBounds.y;

    glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, nearPlaneLight, farPlaneLight);

    // Combine for snapping
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Calculate origin in light space (projection * view * origin)
    glm::vec4 origin = lightSpaceMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    origin /= origin.w;

    // Compute texel size in light space (assuming orthographic width)
    float texelSizeX = (right - left) / ShadowMapSize;
    float texelSizeY = (top - bottom) / ShadowMapSize;

    // Snap the origin to the nearest texel
    float offsetX = std::round(origin.x / texelSizeX) * texelSizeX - origin.x;
    float offsetY = std::round(origin.y / texelSizeY) * texelSizeY - origin.y;

    // Create offset matrix to snap the projection (only translate x,y)
    glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), glm::vec3(offsetX, offsetY, 0.0f));

    // Apply offset to lightProjection
    lightProjection = offsetMat * lightProjection;

    return lightProjection * lightView;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(int i)
{
    //if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions          // texture Coords
            -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, // bottom left
            -1.0f,  0.0f, 0.0f,   1.0f, 0.0f, // top left
             0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // bottom right
             0.0f,  0.0f, 0.0f,   0.0f, 0.0f, // top right
        };

		// Scale quad vertices to fit the screen aspect ratio and push it to the bottom left corner
		int height = Engine::GetCurentScreenHeight();
		int width = Engine::GetCurentScreenWidth();
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

        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}