#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "entity.hpp"

class World;

// Records structural changes so systems can request them while iterating a pool,
// and applies them at a point where nothing is mid-iteration.
class CommandBuffer {
public:
    using Command = std::function<void(World&)>;

    void record(Command command) { commands.push_back(std::move(command)); }

    template <typename T>
    void add(Entity e, T component);

    template <typename Tag>
    void addTag(Entity e);

    template <typename T>
    void remove(Entity e);

    void destroy(Entity e);

    void flush(World& world);

    bool empty() const { return commands.empty(); }

    void create(std::function<void(World&, Entity)> setup);

private:
    std::vector<Command> commands;
};
