#pragma once
#include <glm/glm.hpp>

namespace Camera
{
	void Initialize();
	void UpdateCamera(unsigned int* shaderProgram);
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
	glm::vec3 GetCameraPosition();
	glm::vec3 GetCameraForward();
	glm::vec3 GetCameraRight();
	glm::vec3 GetCameraUp();
	float GetNearPlane();
	float GetFarPlane();
	float GetFOV();
	void SetCameraPosition(glm::vec3 position);
	void SetCameraForward(glm::vec3 forward);
}