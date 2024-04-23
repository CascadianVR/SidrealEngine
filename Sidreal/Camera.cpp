#include "Camera.h"
#include "Engine.h"
#include "Shader.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

glm::vec3 cameraPosition;
glm::vec3 cameraTarget;
glm::vec3 cameraForward;
glm::vec3 worldUp;
glm::vec3 cameraRight;
glm::vec3 cameraUp;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

void Camera::Initialize() {
    cameraPosition = glm::vec3(0.0f, 1.8f, 3.0f);
    cameraTarget = glm::vec3(0.0f, 1.8f, 3.0f) + glm::vec3(0.0f, 0.0f, -3.0f);
    cameraForward = -glm::normalize(cameraPosition - cameraTarget);
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraRight = glm::normalize(glm::cross(worldUp, cameraForward));
    cameraUp = glm::cross(cameraForward, cameraRight);
    view = glm::lookAt(cameraPosition, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
}

void Camera::UpdateCamera(unsigned int* shaderProgram)
{
    // View matrix
    view = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);

    // Perspective projection
    projection = glm::perspective(glm::radians(45.0f), (float)Engine::GetCurentScreenWidth() / (float)Engine::GetCurentScreenHeight(), 0.1f, 100.0f);

    Shader::SetShaderUniformgMatrix4fv(shaderProgram, "view", view);
    Shader::SetShaderUniformgMatrix4fv(shaderProgram, "projection", projection);
}

glm::vec3 Camera::GetCameraPosition()
{
    return cameraPosition;
}

glm::vec3 Camera::GetCameraForward()
{
	return cameraForward;
}

glm::vec3 Camera::GetCameraRight()
{
	return cameraRight;
}

glm::vec3 Camera::GetCameraUp()
{
	return cameraUp;
}

void Camera::SetCameraPosition(glm::vec3 position)
{
	cameraPosition = position;
}

void Camera::SetCameraForward(glm::vec3 forward)
{
    cameraForward = forward;
    cameraRight = glm::normalize(glm::cross(worldUp, cameraForward));
    cameraUp = glm::cross(cameraForward, cameraRight);
}
