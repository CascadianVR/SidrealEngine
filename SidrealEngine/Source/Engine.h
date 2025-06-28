#pragma once
#include "Entity/EntityManager.h"

namespace Engine 
{
	const int WINDOW_WIDTH = 1280;
	const int WINDOW_HEIGHT = 720;

	void Run(const char* startScenePath);
	void ForceRender();
	float GetDeltaTime();
	EntityManager* GetEntityManager();
}