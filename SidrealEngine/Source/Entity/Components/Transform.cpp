#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 EntityTransform::GetModelMatrix(const Transform& transform)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, transform.position);
	model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, transform.scale);
	return model;
}

void EntityTransform::Translate(Transform& transform, const glm::vec3& translation)
{
	transform.position += translation;
}

void EntityTransform::Rotate(Transform& transform, const glm::vec3& rotation)
{
	transform.rotation += rotation;
}

void EntityTransform::Scale(Transform& transform, const glm::vec3& scale)
{
	transform.scale += scale;
}

void EntityTransform::SetTransform(Transform& transform, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
{
}
