#pragma once

#include <functional>
#include <fstream>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "world.hpp"

using json = nlohmann::json;

class SceneSerializer {
public:
    template <typename T>
    void registerComponent(
        const std::string& name,
        std::function<json(const T&)> toJson,
        std::function<T(const json&)> fromJson)
    {
        _registry[name] = {
            // save: check each entity for this component, write if present
            [name, toJson](World& world, json& out) {
                world.forEach<T>([&](Entity e, T& comp) {
                    out[e]["components"][name] = toJson(comp);
                });
            },
            // load: read the json value and add the component to the entity
            [fromJson](World& world, Entity e, const json& value) {
                world.add<T>(e, fromJson(value));
            }
        };
    }

    void save(World& world, const std::string& path) {
        json out;
        // collect per-entity component data from every registered type
        json byEntity;
        for (auto& [name, entry] : _registry)
            entry.save(world, byEntity);

        // flatten the per-entity map into an array
        for (auto& [entityKey, entityData] : byEntity.items())
            out["entities"].push_back(entityData);

        std::ofstream(path) << out.dump(2);
    }

    void load(World& world, const std::string& path) {
        std::ifstream file(path);
        if (!file) throw std::runtime_error("SceneSerializer: cannot open " + path);

        json in = json::parse(file);

        // destroy all existing entities first
        std::vector<Entity> existing;
        // collect via a known component is unreliable — iterate generations instead
        // We expose a clear() on world or just re-construct; for now require caller
        // to pass a fresh world, which is the cleanest contract.

        for (auto& entityData : in["entities"]) {
            Entity e = world.createEntity();
            for (auto& [name, value] : entityData["components"].items()) {
                auto it = _registry.find(name);
                if (it != _registry.end())
                    it->second.load(world, e, value);
                // unregistered component names are silently skipped
            }
        }
    }

private:
    struct Entry {
        std::function<void(World&, json&)>              save;
        std::function<void(World&, Entity, const json&)> load;
    };

    std::unordered_map<std::string, Entry> _registry;
};
