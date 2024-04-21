#pragma once

#include <glm/fwd.hpp>

namespace Camera
{
	void Initialize();

	void UpdateCamera(unsigned int* shaderProgram);

	glm::vec3 GetCameraPosition();

	glm::vec3 GetCameraForward();

	glm::vec3 GetCameraRight();

	glm::vec3 GetCameraUp();

	void SetCameraPosition(glm::vec3 position);

	void SetCameraForward(glm::vec3 forward);

}