#pragma once

namespace Components
{
	struct RenderMeshData
	{
		unsigned int vao = -1;
		unsigned int vbo = -1;
		unsigned int ebo = -1;
		unsigned int texture = -1;
	};

	struct RenderData
	{
		std::vector<RenderMeshData> renderMeshData;
	};
}