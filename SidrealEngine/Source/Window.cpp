#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Window.h"
#include <Engine.h>
#include <iostream>

void SizeCallback(GLFWwindow* window, int width, int height);
void RefreshCallback(GLFWwindow* window);

const int MSAA_SAMPLES = 4;
unsigned int screenWidth;
unsigned int screenHeight;

GLFWwindow* Window::CreateWindow()
{
	// Initialize GLFW with OpenGL Version 4.6 and core profile (less backwards compatability)
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	// Attempt to create a GLFW window and set it to the current context
	GLFWwindow* window = glfwCreateWindow(Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT, "SidrealEngine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// Set vsync (0 is off, 1 is on)
	glfwSwapInterval(0);

	// Initialize GLAD before calling any OpenGL funtions since it manages function pointers for OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	// Set viewport width (same size as window) and register callback to update whe nuser adjusts window size
	glViewport(0, 0, Engine::WINDOW_WIDTH, Engine::WINDOW_HEIGHT);
	screenWidth = Engine::WINDOW_WIDTH;
	screenHeight = Engine::WINDOW_HEIGHT;

	glfwSetFramebufferSizeCallback(window, SizeCallback);
	glfwSetWindowRefreshCallback(window, RefreshCallback);

	return window;
}

void Window::ShowWindow(GLFWwindow* window)
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

void SizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

void RefreshCallback(GLFWwindow* window)
{
	Engine::ForceRender();
	glfwSwapBuffers(window);
	glFinish();
}

int Window::GetCurentScreenWidth()
{
	return screenWidth;
}

int Window::GetCurentScreenHeight()
{
	return screenHeight;
}