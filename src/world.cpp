#include "ecs/world.hpp"

Entity World::createEntity() {
    return nextEntity++;
}

void World::destroyEntity(Entity e) {
    storage.removeAll(e);
}

bool World::isValid(Entity e) const {
    return e != INVALID_ENTITY && e < nextEntity;
}

void World::registerSystem(System system) {
    systems.push_back(system);
}

void World::update() {
    for (auto& system : systems)
        system(*this);
}
