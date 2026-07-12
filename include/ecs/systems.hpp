#pragma once

#include <iostream>
#include <vector>
#include "world.hpp"
#include "components.hpp"

inline void movementSystem(World& world) {
    world.view<Position, Velocity>([](Entity e, Position& pos, Velocity& vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });
}

inline void damageSystem(World& world) {
    std::vector<Entity> dead;
    world.view<Health, Damage>([&](Entity e, Health& hp, Damage& dmg) {
        if (!world.hasTag<Enemy>(e)) return;
        hp.hp -= dmg.amount;
        if (hp.hp <= 0.0f) dead.push_back(e);
    });
    for (Entity e : dead) world.destroyEntity(e);
}

inline void healthPrintSystem(World& world) {
    world.forEach<Health>([](Entity e, Health& hp) {
        std::cout << "Entity " << e << " hp: " << hp.hp << "\n";
    });
}
