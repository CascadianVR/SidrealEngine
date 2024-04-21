#pragma once

namespace Engine 
{
	const int WINDOW_WIDTH = 1280;
	const int WINDOW_HEIGHT = 720;

	void Run();
	int GetCurentScreenWidth();
	int GetCurentScreenHeight();
	float GetDeltaTime();
}