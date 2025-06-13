#pragma once
#include <queue>
#include "Components/Transform.h"

typedef int Entity;
const int MAX_ENTITIES = 1024;

class EntityManager {
private:
    std::queue<Entity> available;
    int nextEntity = 0;

public:
    std::vector <EntityTransform::Transform> transforms;
    bool hasTransform[MAX_ENTITIES] = { false };

    std::vector<Model> models;
    bool hasModel[MAX_ENTITIES] = { false };

    EntityManager() {
        for (int i = 0; i < MAX_ENTITIES; ++i) {
            available.push(i);
        }

        transforms.resize(MAX_ENTITIES);
        models.resize(MAX_ENTITIES);
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