#include "ecs/world.hpp"
#include "ecs/entity.hpp"
#include <cstdint>

Entity World::createEntity() {
    uint32_t index;
	uint8_t gen;

	if (!freeList.empty()){
		index = freeList.front();
		freeList.pop();
		gen = generations[index];
	} else {
		index = nextIndex++;
		generations.push_back(0);
		gen = 0;
	}

	return makeEntity(index, gen);
}

void World::destroyEntity(Entity e) {
    if (!isValid(e)) return;

	uint32_t index = entityIndex(e);
	generations[index]++;
	storage.removeAll(index);
	freeList.push(index);
}

bool World::isValid(Entity e) const {
    if (e == INVALID_ENTITY) return false;
	uint32_t index = entityIndex(e);
	if (index >= generations.size()) return false;
	return entityGeneration(e) == generations[index];
}

void World::registerSystem(System system) {
    systems.push_back(system);
}

void World::update() {
    for (auto& system : systems)
        system(*this);
}
