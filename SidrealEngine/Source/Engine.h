#pragma once
#include "Scene.h"
#include "Entity/EntityManager.h"

namespace Engine 
{
	const int WINDOW_WIDTH = 1280;
	const int WINDOW_HEIGHT = 720;

	void Run(const char* startScenePath);
	int GetCurentScreenWidth();
	int GetCurentScreenHeight();
	float GetDeltaTime();
	EntityManager* GetEntityManager();
}