#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace EntityTransform
{
	struct Transform
	{
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	};

	glm::mat4 GetModelMatrix(const Transform& transform);
	void Translate(Transform& transform, const glm::vec3& translation);
	void Rotate(Transform& transform, const glm::vec3& rotation);
	void Scale(Transform& transform, const glm::vec3& scale);
	void SetTransform(Transform& transform, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
}