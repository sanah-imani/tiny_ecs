#pragma once

#include "entity.hpp"
#include <unordered_map>

struct IComponentArray {
    virtual ~IComponentArray() = default;
    virtual void remove(Entity e) = 0;
};

template <typename T> 
struct ComponentArray: IComponentArray {
    std::unordered_map<Entity, T> data;

    void remove(Entity e) override {data.erase(e);}
};
