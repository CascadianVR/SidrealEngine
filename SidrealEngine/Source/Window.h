#pragma once
#include <GLFW/glfw3.h>

namespace Window
{
	GLFWwindow* CreateWindow();
	void ShowWindow(GLFWwindow* window);
	int GetCurentScreenWidth();
	int GetCurentScreenHeight();
}