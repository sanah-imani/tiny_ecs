#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include "entity.hpp"
#include "component_array.hpp"

class ComponentStorage {
public:
    template <typename T>
    void add(Entity e, T component) {
        getArray<T>().data[e] = component;
    }

    template <typename T>
    T* get(Entity e) {
        auto& arr = getArray<T>();
        auto it = arr.data.find(e);
        if (it == arr.data.end()) return nullptr;
        return &it->second;
    }

    template <typename T>
    bool has(Entity e) {
        auto& arr = getArray<T>();
        return arr.data.find(e) != arr.data.end();
    }

    template <typename T>
    void remove(Entity e) {
        getArray<T>().data.erase(e);
    }

    void removeAll(Entity e) {
        for (auto& [key, array] : arrays)
            array->remove(e);
    }

    template <typename T, typename Func>
    void forEach(Func fun) {
        for (auto& [entity, component] : getArray<T>().data)
            fun(entity, component);
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> arrays;

    template <typename T>
    ComponentArray<T>& getArray() {
        auto key = std::type_index(typeid(T));
        if (arrays.find(key) == arrays.end())
            arrays[key] = std::make_unique<ComponentArray<T>>();
        return static_cast<ComponentArray<T>&>(*arrays[key]);
    }
};
