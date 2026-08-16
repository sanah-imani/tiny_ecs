#include "ecs/command_buffer.hpp"

#include <vector>

#include "ecs/entity.hpp"
#include "ecs/world.hpp"

void CommandBuffer::destroy(Entity e) {
    commands.emplace_back([e](World& w) { w.destroyEntity(e); });
}

void CommandBuffer::flush(World& world) {
    // Commands may record further commands, so drain in passes rather than
    // iterating a vector that can reallocate underneath the loop.
    while (!commands.empty()) {
        std::vector<Command> batch;
        batch.swap(commands);
        for (auto& command : batch) command(world);
    }
}

void CommandBuffer::create(std::function<void(World&, Entity)> setup){
    record([s = std::move(setup)] (World& w){
        Entity e = w.createEntity();
        s(w, e);
    });
}