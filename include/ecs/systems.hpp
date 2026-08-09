#pragma once

#include <iostream>

#include "components.hpp"
#include "world.hpp"

inline void movementSystem(World& world) {
    world.view<Position, Velocity>([](Entity, Position& pos, Velocity& vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });
}

inline void damageSystem(World& world) {
    CommandBuffer& cmd = world.commands();
    world.view<Health, Damage>([&](Entity e, Health& hp, Damage& dmg) {
        if (!world.hasTag<Enemy>(e)) return;
        hp.hp -= dmg.amount;
        world.events().emit(DamageTaken{e, dmg.amount});
        if (hp.hp <= 0.0f) {
            world.events().enqueue(EntityDied{e});
            cmd.destroy(e);
        }
    });
}

inline void healthPrintSystem(World& world) {
    world.forEach<Health>([](Entity e, Health& hp) {
        std::cout << "Entity " << e << " hp: " << hp.hp << "\n";
    });
}
