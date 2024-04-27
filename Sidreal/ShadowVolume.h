#pragma once
#include <glm/ext/vector_float3.hpp>

namespace ShadowVolume
{
	typedef struct Surface {
		glm::vec3 vertices[4];
		float matrix[9];

		float s_dist, t_dist;
	};

	typedef struct Cube {
		struct Surface* surfaces[6];
		glm::vec3 position;
	};

	void RenderSurfaceShadowVolume(Surface* surf, glm::vec3 surf_pos, glm::vec3 light_pos);
	void RenderCubeShadow(Cube* c);
}