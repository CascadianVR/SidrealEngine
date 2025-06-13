#include "Scene.h"

Scene::SceneData* activeScene;

Scene::SceneData* Scene::GetActiveScene()
{
	return activeScene;
}

void Scene::SetActiveScene(SceneData* scene)
{
	activeScene = scene;
}
