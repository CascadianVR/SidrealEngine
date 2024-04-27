#pragma once

namespace Renderer
{
	void Initialize();
	void Render();
	unsigned int* GetShaderProgram();
	void LoadShaders(bool reload);
	void Cleanup();
}