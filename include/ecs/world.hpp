#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

#include "component_storage.hpp"
#include "components.hpp"
#include "entity.hpp"

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

    template <typename Tag>
    void addTag(Entity e) {
        if (!isValid(e)) return;
        storage.add<Tag>(entityIndex(e), Tag{});
    }

    template <typename Tag>
    bool hasTag(Entity e) {
        return storage.has<Tag>(entityIndex(e));
    }

    template <typename... Ts, typename Func>
    void view(Func func) {
        static_assert(sizeof...(Ts) > 0, "view requires at least one component type");
        const std::array<size_t, sizeof...(Ts)> sizes{storage.size<Ts>()...};
        const auto pivot = static_cast<size_t>(
            std::min_element(sizes.begin(), sizes.end()) - sizes.begin());
        dispatch<Ts...>(pivot, func, std::index_sequence_for<Ts...>{});
    }

private:
    uint32_t nextIndex = 1;
    std::vector<uint8_t> generations;
    std::queue<uint32_t> freeList;
    ComponentStorage storage;
    std::vector<System> systems;

    Entity entityAt(EntityIndex index) const { return makeEntity(index, generations[index]); }

    template <typename Pivot, typename... Ts, typename Func>
    void driveBy(Func& func) {
        storage.forEach<Pivot>([&](EntityIndex index, Pivot&) {
            auto ptrs = std::make_tuple(storage.get<Ts>(index)...);
            const bool complete = std::apply([](auto*... p) { return ((p != nullptr) && ...); }, ptrs);
            if (complete)
                std::apply([&](auto*... p) { func(entityAt(index), *p...); }, ptrs);
        });
    }

    // pivot is a runtime choice, so fan out over every compile-time position and
    // run only the branch that matches.
    template <typename... Ts, typename Func, size_t... Is>
    void dispatch(size_t pivot, Func& func, std::index_sequence<Is...>) {
        ((Is == pivot ? driveBy<std::tuple_element_t<Is, std::tuple<Ts...>>, Ts...>(func) : void()), ...);
    }
};
