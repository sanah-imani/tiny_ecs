#pragma once

#include <cstdint>
#include <queue>
#include <vector>
#include <functional>
#include "entity.hpp"
#include "components.hpp"
#include "component_storage.hpp"

class World {
public:
    using System = std::function<void(World&)>;

    World() { generations.push_back(0); }  // reserve slot 0 for INVALID_ENTITY

    Entity createEntity();
    void destroyEntity(Entity e);
    bool isValid(Entity e) const;

    void registerSystem(System system);
    void update();

    template <typename T>
    void add(Entity e, T component) {
        if (!isValid(e)) return;
        storage.add<T>(entityIndex(e), component);
    }

    template <typename T>
    T* get(Entity e) {
        return storage.get<T>(entityIndex(e));
    }

    template <typename T>
    bool has(Entity e) {
        return storage.has<T>(entityIndex(e));
    }

    template <typename T>
    void remove(Entity e) {
        storage.remove<T>(entityIndex(e));
    }

    template <typename T, typename Func>
    void forEach(Func func) {
        storage.forEach<T>(func);
    }

    template <typename A, typename B, typename Func>
    void view(Func func){
        storage.forEach<A>([&](uint32_t index, A& a){
            if (auto* b = storage.get<B>(index)){
                Entity e = makeEntity(index, generations[index]);
                func(e, a, *b);
            }
        });
    }

private:
    uint32_t nextIndex = 1;
    std::vector<uint8_t> generations;
    std::queue<uint32_t> freeList;
    ComponentStorage storage;
    std::vector<System> systems;
};
