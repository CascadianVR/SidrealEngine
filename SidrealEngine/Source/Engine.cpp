#include <GLFW/glfw3.h>
#include "Window.h"
#include "Engine.h"
#include "Input.h"
#include "Renderer/Renderer.h"
#include "Utils/Timer.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Entity/EntityManager.h"
#include "Entity/Systems.h"
#include "UI/ImGuiOverlay.h"

GLFWwindow* window = NULL;
EntityManager* _entityManager = nullptr;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void Engine::Run(const char* startScenePath)
{
	window = Window::CreateWindow();

	EntityManager entityManager;
	_entityManager = &entityManager;


	//AssetPacker::AddSceneToPack(startScenePath);
	//AssetPacker::PackAllAssets();

	//AssetPacker::LoadAllPackedAssets("assets.sap", _entityManager);
	Scene::SceneData* scene = AssetManager::LoadSceneFromJSON(startScenePath, _entityManager);
	Scene::SetActiveScene(scene);

	Window::ShowWindow(window);
	Renderer::Initialize();
	Input::Initialize(window);
	ImGuiOverlay::Initialize(window);

	// Keep the main window open
	while (!glfwWindowShouldClose(window))
	{	
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// Poll GLFW events
		glfwPollEvents();

		// Process input
		Input::ProcessInput(window);
		
		// Render the scene
		RenderSystem(*_entityManager);

		// Render ImGui
		ImGuiOverlay::Render(deltaTime);

		{
			ScopedTimer swapBuffersTimer("Swap Buffers");
			glfwSwapBuffers(window);
		}
	}
	
	// Destrow windows and free resources
	glfwTerminate();
	return;
}

void Engine::ForceRender()
{
	// Render the scene
	RenderSystem(*_entityManager);

	// Render ImGui
	ImGuiOverlay::Render(deltaTime);
}

float Engine::GetDeltaTime()
{
	return deltaTime;
}

EntityManager* Engine::GetEntityManager()
{
	return _entityManager;
}