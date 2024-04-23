#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Engine.h"
#include "Input.h"
#include "Renderer.h"

void InitializeEngine();
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

GLFWwindow* window = NULL;
int screenWidth = 0;
int screenHeight = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void Engine::Run() 
{
	InitializeEngine();

	// Keep the main window open
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		Input::ProcessInput(window);

		Renderer::Render();

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
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
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