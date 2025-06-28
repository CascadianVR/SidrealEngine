#pragma once
#include <glm/ext/vector_float3.hpp>

namespace MathUtils
{
	glm::vec3 LerpVec3(const glm::vec3& start, const glm::vec3& end, float t);

	float* Vec3toFloat3(glm::vec3 vec);
	float* Vec4toFloat4(glm::vec4 vec);
	glm::vec3 FloorVector(const glm::vec3& vec);
}