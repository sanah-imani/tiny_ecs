#pragma once

#include "world.hpp"
#include "components.hpp"

inline void movementSystem(World& world) {
    world.view<Position, Velocity>([](Entity e, Position& pos, Velocity& vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });
}
