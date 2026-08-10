#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "component_array.hpp"
#include "entity.hpp"

struct IObserverList { virtual ~IObserverList() = default; };

template <typename T>
struct ObserverList : IObserverList {
    std::vector<std::function<void(Entity, T&)>> onAdd;
    std::vector<std::function<void(Entity)>>     onRemove;
};

class ComponentStorage {
public:
    template <typename T>
    void add(Entity e, T component) {
        getArray<T>().data.insert_or_assign(e, std::move(component));
        if (auto* obs = findObservers<T>()) {
            T& stored = *get<T>(e);
            for (auto& cb : obs->onAdd) cb(e, stored);
        }
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
    size_t size() const {
        const auto* arr = findArray<T>();
        return arr ? arr->data.size() : 0;
    }

    template <typename T>
    void remove(Entity e) {
        if (auto* arr = findArray<T>()) {
            if (arr->data.count(e)) {
                if (auto* obs = findObservers<T>())
                    for (auto& cb : obs->onRemove) cb(e);
                arr->data.erase(e);
            }
        }
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

    template <typename T>
    void onAdd(std::function<void(Entity, T&)> cb) {
        getObservers<T>().onAdd.push_back(std::move(cb));
    }

    template <typename T>
    void onRemove(std::function<void(Entity)> cb) {
        getObservers<T>().onRemove.push_back(std::move(cb));
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> arrays;
    std::unordered_map<std::type_index, std::unique_ptr<IObserverList>>   observers;

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

    template <typename T>
    ObserverList<T>* findObservers() {
        auto it = observers.find(std::type_index(typeid(T)));
        return it == observers.end() ? nullptr
                                     : static_cast<ObserverList<T>*>(it->second.get());
    }

    template <typename T>
    ObserverList<T>& getObservers() {
        auto key = std::type_index(typeid(T));
        auto it = observers.find(key);
        if (it == observers.end())
            it = observers.emplace(key, std::make_unique<ObserverList<T>>()).first;
        return static_cast<ObserverList<T>&>(*it->second);
    }
};
