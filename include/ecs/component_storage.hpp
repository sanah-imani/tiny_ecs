#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "component_array.hpp"
#include "entity.hpp"

class ComponentStorage {
public:
    template <typename T>
    void add(Entity e, T component) {
        getArray<T>().data.insert_or_assign(e, std::move(component));
    }

    template <typename T, typename... Args>
    T& emplace(Entity e, Args&&... args) {
        auto result = getArray<T>().data.insert_or_assign(e, T{std::forward<Args>(args)...});
        return result.first->second;
    }

    template <typename T>
    const T* get(Entity e) const {
        const auto* arr = findArray<T>();
        if (!arr) return nullptr;
        auto it = arr->data.find(e);
        return it == arr->data.end() ? nullptr : &it->second;
    }

    template <typename T>
    T* get(Entity e) {
        return const_cast<T*>(std::as_const(*this).get<T>(e));
    }

    template <typename T>
    bool has(Entity e) const {
        return get<T>(e) != nullptr;
    }

    template <typename T>
    void remove(Entity e) {
        if (auto* arr = findArray<T>()) arr->data.erase(e);
    }

    void removeAll(Entity e) {
        for (auto& [key, array] : arrays) array->remove(e);
    }

    template <typename T, typename Func>
    void forEach(Func fun) {
        auto* arr = findArray<T>();
        if (!arr) return;
        for (auto& [entity, component] : arr->data) fun(entity, component);
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> arrays;

    template <typename T>
    const ComponentArray<T>* findArray() const {
        auto it = arrays.find(std::type_index(typeid(T)));
        if (it == arrays.end()) return nullptr;
        return static_cast<const ComponentArray<T>*>(it->second.get());
    }

    template <typename T>
    ComponentArray<T>* findArray() {
        return const_cast<ComponentArray<T>*>(std::as_const(*this).findArray<T>());
    }

    template <typename T>
    ComponentArray<T>& getArray() {
        auto key = std::type_index(typeid(T));
        auto it = arrays.find(key);
        if (it == arrays.end()) it = arrays.emplace(key, std::make_unique<ComponentArray<T>>()).first;
        return static_cast<ComponentArray<T>&>(*it->second);
    }
};
