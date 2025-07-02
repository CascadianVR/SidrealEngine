#pragma once
#include <queue>
#include <vector>
#include <Renderer/Model.h>
#include <Entity/Components/Transform.h>
#include <Entity/Components/RenderData.h>

using namespace Components;

typedef unsigned int Entity;
const unsigned int MAX_ENTITIES = 1024;

class EntityManager {
private:
    std::queue<Entity> available;
    unsigned int nextEntity = 0;

public:
    std::vector<Transform> transforms;
    bool hasTransform[MAX_ENTITIES] = { false };

    std::vector<Model> models;
    bool hasModel[MAX_ENTITIES] = { false };

    std::vector<RenderData> renderData;
    bool hasRenderData[MAX_ENTITIES] = { false };

    EntityManager() {
        for (unsigned int i = 0; i < MAX_ENTITIES; ++i) {
            available.push(i);
        }

        transforms.resize(MAX_ENTITIES);
        models.resize(MAX_ENTITIES);
        renderData.resize(MAX_ENTITIES);
    }

    Entity CreateEntity() {
        if (available.empty()) return -1;
        Entity id = available.front();
        available.pop();
        return id;
    }

    void DestroyEntity(Entity entity) {
        available.push(entity);
    }
};