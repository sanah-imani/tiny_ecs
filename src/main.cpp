#include <iostream>
#include "ecs/components.hpp"
#include "ecs/world.hpp"
#include "ecs/systems.hpp"

int main() {
    World world;

    world.registerSystem(movementSystem);
	world.registerSystem(healthPrintSystem);
	world.registerSystem(damageSystem);

    Entity e = world.createEntity();
    world.add<Position>(e, {0, 0});
    world.add<Velocity>(e, {1, 0});

	Entity enemy = world.createEntity();
	world.add<Health>(enemy, {100.0f, 100.0f});
	world.add<Damage>(enemy, {10.0f});
	world.addTag<Enemy>(enemy);

    for (int i = 0; i < 15; i++) {
        world.update();
        auto* pos = world.get<Position>(e);
        std::cout << "Frame " << i << " - pos: ("
                  << pos->x << ", " << pos->y << ")\n";
    }
    return 0;
}
