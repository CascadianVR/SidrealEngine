#include <cmath>  // For std::floor
#include <glm/glm.hpp>
#include "MathUtils.h"


glm::vec3 MathUtils::LerpVec3(const glm::vec3& start, const glm::vec3& end, float t) {
	return start + t * (end - start);
}

float* MathUtils::Vec3toFloat3(glm::vec3 vec) {
	float* arr = new float[3];
	arr[0] = vec.x;
	arr[1] = vec.y;
	arr[2] = vec.z;
	return arr;
}

float* MathUtils::Vec4toFloat4(glm::vec4 vec) {
	float* arr = new float[4];
	arr[0] = vec.x;
	arr[1] = vec.y;
	arr[2] = vec.z;
	arr[3] = vec.w;
	return arr;
}

glm::vec3 FloorVector(const glm::vec3& vec)
{
	return glm::vec3(
		std::floor(vec.x),
		std::floor(vec.y),
		std::floor(vec.z)
	);
}