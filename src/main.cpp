#include <iostream>
#include "ecs/world.hpp"
#include "ecs/systems.hpp"

int main() {
    World world;

    world.registerSystem(movementSystem);

    Entity e = world.createEntity();
    world.add<Position>(e, {0, 0});
    world.add<Velocity>(e, {1, 0});

    for (int i = 0; i < 5; i++) {
        world.update();
        auto* pos = world.get<Position>(e);
        std::cout << "Frame " << i << " — pos: ("
                  << pos->x << ", " << pos->y << ")\n";
    }
    return 0;
}
