#pragma once

namespace Renderer
{
	struct RenderData
	{
		float* vertexBuffer = nullptr; // Pointer to vertex buffer data
		unsigned int* indexBuffer = nullptr; // Pointer to index buffer data
		unsigned int vertexCount = 0; // Number of vertices
		unsigned int indexCount = 0; // Number of indices
		RenderData() = default;
		RenderData(float* vb, unsigned int* ib, unsigned int vCount, unsigned int iCount)
			: vertexBuffer(vb), indexBuffer(ib), vertexCount(vCount), indexCount(iCount) {}
	};
}