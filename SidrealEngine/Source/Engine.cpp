#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Engine.h"
#include "Input.h"
#include "Renderer/Renderer.h"
#include "Timer.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Entity/EntityManager.h"
#include "Entity/Systems.h"

void InitializeGlfw();
void ShowWindow();
void InitializeImGui();
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ImGuiStatisticsWindow();
void ImGuiControlsWindow();

const int MSAA_SAMPLES = 4;

GLFWwindow* window = NULL;
int screenWidth = 0;
int screenHeight = 0;

// Timer variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float averageTime[60];
int frameNumber = 0;
float average = 0.0f;

EntityManager* _entityManager = nullptr;

ImGuiIO io;

void Engine::Run(const char* startScenePath)
{
	InitializeGlfw();

	EntityManager entityManager;
	_entityManager = &entityManager;

	Scene::SceneData* scene = AssetManager::LoadSceneFromJSON(startScenePath, _entityManager);
	Scene::SetActiveScene(scene);

	ShowWindow();
	Renderer::Initialize();
	Input::Initialize(window);
	InitializeImGui();

	// Keep the main window open
	while (!glfwWindowShouldClose(window))
	{	
		TimerRegistry::Reset();
		{
			ScopedTimer mainLoopTimer("Main Loop Timer");

			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

		
			// Poll GLFW events
			glfwPollEvents();

			// Process input
			Input::ProcessInput(window);
		
			// Render the scene
			RenderSystem(entityManager);

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// Your ImGui UI code
			ImGuiStatisticsWindow();
			ImGuiControlsWindow();

			// Finalize and render ImGui
			ImGui::Render();
			glViewport(0, 0, Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			

			// Swap buffers and poll events
			glfwSwapBuffers(window);
		}
	}
	
	// Destrow windows and free resources
	glfwTerminate();
	return;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

std::unordered_map<std::string, double> timings;
void ImGuiStatisticsWindow()
{

	ImGui::Begin("Scene Statistics", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowSize(ImVec2(250, 140));
	ImGui::SetWindowPos(ImVec2(0, 80));

	if (frameNumber < 60)
	{
		averageTime[frameNumber] = deltaTime;
		frameNumber++;
	}
	else
	{
		float sum = 0;
		for (int i = 0; i < 60; i++)
		{
			sum += averageTime[i];
		}
		average = sum / 60;
		frameNumber = 0;
		timings = TimerRegistry::GetTimings();
	}

	ImGui::Text("Main Loop: %.2f ms", average * 1000);
	ImGui::Text("Main Loop: %.0f fps", 1 / average);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Timers:");

	for (auto& timer : timings)
	{
		ImGui::Text("%s %.2f ms", timer.first.c_str(), timer.second);
	}

	ImGui::End();
}

void ImGuiControlsWindow()
{
	ImGui::Begin("Controls", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowSize(ImVec2(250, 140));
	ImGui::SetWindowPos(ImVec2(0, 0));

	ImGui::Text("Light Direction:");
	float* dir = Renderer::GetLightDirection();
	ImGui::SliderFloat3("Light Direction X", dir, -1.0f, 1.0f);
	Renderer::SetLightDirection(dir);

	ImGui::End();
}

int Engine::GetCurentScreenWidth()
{
	return screenWidth;
}

int Engine::GetCurentScreenHeight()
{
	return screenHeight;
}

float Engine::GetDeltaTime()
{
	return deltaTime;
}

EntityManager* Engine::GetEntityManager()
{
	return _entityManager;
}

void InitializeGlfw()
{
	// Initialize GLFW with OpenGL Version 4.6 and core profile (less backwards compatability)
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	// Attempt to create a GLFW window and set it to the current context
	window = glfwCreateWindow(Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT, "SidrealEngine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	// Set vsync (0 is off, 1 is on)
	glfwSwapInterval(0);

	// Initialize GLAD before calling any OpenGL funtions since it manages function pointers for OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	// Set viewport width (same size as window) and register callback to update whe nuser adjusts window size
	screenWidth = Engine::WINDOW_WIDTH;
	screenHeight = Engine::WINDOW_HEIGHT;
	glViewport(0, 0, Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
}

void ShowWindow()
{
	if (window == NULL)
	{
		std::cout << "GLFW window is not initialized." << std::endl;
		return;
	}
	// Show the GLFW window
	glfwShowWindow(window);
	glfwFocusWindow(window);
	glfwSetWindowTitle(window, "SidrealEngine");
}


void InitializeImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls    
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls    

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}