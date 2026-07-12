#pragma once

#include <vector>
#include <functional>
#include "entity.hpp"
#include "components.hpp"
#include "component_storage.hpp"

class World {
public:
    using System = std::function<void(World&)>;

    World() = default;

    Entity createEntity();
    void destroyEntity(Entity e);
    bool isValid(Entity e) const;

    void registerSystem(System system);
    void update();

    template <typename T>
    void add(Entity e, T component) {
        if (!isValid(e)) return;
        storage.add<T>(e, component);
    }

    template <typename T>
    T* get(Entity e) {
        return storage.get<T>(e);
    }

    template <typename T>
    bool has(Entity e) {
        return storage.has<T>(e);
    }

    template <typename T>
    void remove(Entity e) {
        storage.remove<T>(e);
    }

    template <typename T, typename Func>
    void forEach(Func func) {
        storage.forEach<T>(func);
    }

private:
    Entity nextEntity = 1;
    ComponentStorage storage;
    std::vector<System> systems;
};
