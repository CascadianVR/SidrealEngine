#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <glm/geometric.hpp>

#include "Engine.h"
#include "Input.h"
#include "Camera.h"
#include "MathUtils.h"

void UpdateCameraPosition(GLFWwindow* window);
void UpdateCameraRotation();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

const float cameraMoveSpeed = 0.03f;
glm::vec3 cameraForwardTarget;

const float cameraLookSensitivity = 0.1f;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;

glm::vec3 cameraPosTarget;

void Input::Initialize(GLFWwindow* window)
{
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cameraForwardTarget = Camera::GetCameraForward();
    cameraPosTarget = Camera::GetCameraPosition();
}

void Input::ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    UpdateCameraPosition(window);
    UpdateCameraRotation();
}

void UpdateCameraPosition(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosTarget += cameraMoveSpeed * Camera::GetCameraForward();
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPosTarget += cameraMoveSpeed * Camera::GetCameraForward() * -1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPosTarget += cameraMoveSpeed * Camera::GetCameraRight();
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPosTarget += cameraMoveSpeed * Camera::GetCameraRight() * -1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cameraPosTarget += cameraMoveSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cameraPosTarget += cameraMoveSpeed * glm::vec3(0.0f, 1.0f, 0.0f) * -1.0f;
    }

    Camera::SetCameraPosition(MathUtils::LerpVec3(Camera::GetCameraPosition(), cameraPosTarget, Engine::GetDeltaTime() * 10.0f));
}

void UpdateCameraRotation()
{
    glm::vec3 newCamForward = MathUtils::LerpVec3(Camera::GetCameraForward(), cameraForwardTarget, Engine::GetDeltaTime() * 20.0f);
    Camera::SetCameraForward(glm::normalize(newCamForward));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
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
    glm::normalize(cameraForwardTarget);
}

