#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include "Timer.h"
#include<windows.h>

void InitializeEngine();
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ImGuiStatisticsWindow();

GLFWwindow* window = NULL;
int screenWidth = 0;
int screenHeight = 0;

// Timer variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float averageTime[60];
int frameNumber = 0;
float average = 0.0f;

std::unordered_map<const char*, Timer::Timer> timers;

ImGuiIO io;

void Engine::Run() 
{
	// Initialize GLFW, GLAD, and create a window
	InitializeEngine();

	// Keep the main window open
	while (!glfwWindowShouldClose(window))
	{
		Timer::Start("Full Loop");

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Poll GLFW events
		glfwPollEvents();

		// Process input
		Input::ProcessInput(window);

		// Render the scene
		Renderer::Render();

		Timer::Stop("Full Loop");

		// Render Dear ImGui
		ImGuiStatisticsWindow();
		ImGui::Render();
		glViewport(0, 0, Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll IO events
		glfwSwapBuffers(window);

	}

	// Destrow windows and free resources
	glfwTerminate();
	return;
}

void InitializeEngine() 
{
	// Initialize GLFW with OpenGL Version 4.6 and core profile (less backwards compatability)
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Attempt to create a GLFW window and set it to the current context
	window = glfwCreateWindow(Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT, "SidrealEngine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	// Disable vsync
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

	// Initialize renderer
	Renderer::Initialize();

	// Initialize input
	Input::Initialize(window);

	// Setup Dear ImGui context

	timers = *Timer::GetTimerMap();

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

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

void ImGuiStatisticsWindow()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	// Render Dear ImGui Window
	ImGui::NewFrame();
	{
		ImGui::Begin("Scene Statistics", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::SetWindowSize(ImVec2(250, 140));
		ImGui::SetWindowPos(ImVec2(0, 0));

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
			timers = *Timer::GetTimerMap();
		}

		ImGui::Text("Main Loop: %.2f ms", average * 1000);
		ImGui::Text("Main Loop: %.0f fps", 1 / average);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Timers:");

		for (auto& timer : timers)
		{
			ImGui::Text("%s %.2f ms", timer.first, timer.second.elapsedTimeMs);
		}

		ImGui::End();
	}
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