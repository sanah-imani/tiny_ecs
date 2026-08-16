#include <iostream>
#include "ecs/components.hpp"
#include "ecs/world.hpp"
#include "ecs/systems.hpp"

int main() {
    World world;

    world.registerSystem(movementSystem);
    world.registerSystem(lifetimeSystem);
    world.registerSystem(healthPrintSystem);
    world.registerSystem(damageSystem);

    world.registerSystem([](World &w){
        w.commands().create([](World& w2, Entity bullet){
            w2.add<Position>(bullet, {5.0f, 0.0f});
            w2.add<Velocity>(bullet, {3.0f, 0.0f});
            w2.add<Lifetime>(bullet, {5});

        });
    });

    // Player — moves every frame
    Entity player = world.createEntity();
    world.add<Position>(player, {0, 0});
    world.add<Velocity>(player, {1, 0});
    world.addTag<Player>(player);

    // Enemy — takes 10 damage per frame, dies at frame 10
    Entity enemy = world.createEntity();
    world.add<Health>(enemy, {100.0f, 100.0f});
    world.add<Damage>(enemy, {10.0f});
    world.addTag<Enemy>(enemy);

    // Bullet — moves fast, destroyed after 5 frames
    Entity bullet = world.createEntity();
    world.add<Position>(bullet, {0, 0});
    world.add<Velocity>(bullet, {3, 0});
    world.add<Lifetime>(bullet, {5});

    for (int i = 0; i < 15; i++) {
        world.update();

        auto* pos = world.get<Position>(player);
        std::cout << "Frame " << i << " - player pos: ("
                  << pos->x << ", " << pos->y << ")";

        if (world.isValid(bullet)) {
            auto* bpos = world.get<Position>(bullet);
            std::cout << "  bullet pos: (" << bpos->x << ", " << bpos->y << ")";
        } else {
            std::cout << "  bullet: destroyed";
        }

        std::cout << "\n";
    }
    return 0;
}
