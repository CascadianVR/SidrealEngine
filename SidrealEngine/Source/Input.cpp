#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>
#include "Camera.h"
#include "Engine.h"
#include "Input.h"
#include "Utils/MathUtils.h"
#include "Renderer/Renderer.h"

void UpdateCameraPosition(GLFWwindow* window);
void UpdateCameraRotation();
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

const float cameraMoveSpeed = 5.0f;
const float sprintSpeedMultiplier = 2.5f;
glm::vec3 cameraForwardTarget;

bool mouseLocked = false;
const float cameraLookSensitivity = 0.1f;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;

glm::vec3 cameraPosTarget;

HANDLE fragShaderChanged;

void Input::Initialize(GLFWwindow* window)
{
	// Set mouse position to the center of the window BEFORE setting event 
	// callbacks so it doesn't trigger them on startup, then lock the cursor.
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetCursorPos(window, (float)width / 2, (float)height / 2);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    mouseLocked = true;

	// Get the initial camera position and forward vector
    cameraForwardTarget = Camera::GetCameraForward();
    cameraPosTarget = Camera::GetCameraPosition();
}

void Input::ProcessInput(GLFWwindow* window)
{
    UpdateCameraPosition(window);
    UpdateCameraRotation();
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static bool isEscapeKeyPressed = false;
    static bool isRKeyPressed = false;
    static bool isLeftAltKeyPressed = false;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && !isEscapeKeyPressed)
    {
        isEscapeKeyPressed = true;
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        isEscapeKeyPressed = false;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS && !isRKeyPressed)
    {
        isRKeyPressed = true;
        Renderer::LoadShaders(true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_RELEASE)
    {
        isRKeyPressed = false;
    }

    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS && !isLeftAltKeyPressed)
    {
        isLeftAltKeyPressed = true;
        int mode = glfwGetInputMode(window, GLFW_CURSOR);
        if (mode == GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2, height / 2);
            mouseLocked = false;
        }
        else
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2, height / 2);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouseLocked = true;
        }
    }
    else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
    {
        isLeftAltKeyPressed = false;
    }
}

void UpdateCameraPosition(GLFWwindow* window)
{
    float deltaTime = Engine::GetDeltaTime();

	float currentMoveSpeed = cameraMoveSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
		currentMoveSpeed *= sprintSpeedMultiplier;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        currentMoveSpeed /= sprintSpeedMultiplier;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosTarget += currentMoveSpeed * Camera::GetCameraForward() * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPosTarget += currentMoveSpeed * Camera::GetCameraForward() * -1.0f * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPosTarget += currentMoveSpeed * Camera::GetCameraRight() * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPosTarget += currentMoveSpeed * Camera::GetCameraRight() * -1.0f * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cameraPosTarget += currentMoveSpeed * glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cameraPosTarget += currentMoveSpeed * glm::vec3(0.0f, 1.0f, 0.0f) * -1.0f * deltaTime;
    }

    Camera::SetCameraPosition(MathUtils::LerpVec3(Camera::GetCameraPosition(), cameraPosTarget, deltaTime * 10.0f));
}

void UpdateCameraRotation()
{
    if (!mouseLocked)
    {
        cameraForwardTarget = Camera::GetCameraForward();
        Camera::SetCameraForward(cameraForwardTarget);
		return;
	}
    glm::vec3 newCamForward = MathUtils::LerpVec3(Camera::GetCameraForward(), cameraForwardTarget, Engine::GetDeltaTime() * 20.0f);
    Camera::SetCameraForward(glm::normalize(newCamForward));
}

void MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!mouseLocked)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        lastX = static_cast<float>(width) / 2.0f;
        lastY = static_cast<float>(height) / 2.0f;
		return;
	}

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= cameraLookSensitivity;
    yoffset *= cameraLookSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    cameraForwardTarget.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraForwardTarget.y = sin(glm::radians(pitch));
    cameraForwardTarget.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraForwardTarget = glm::normalize(cameraForwardTarget);
}

