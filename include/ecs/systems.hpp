#pragma once

#include <cstddef>
#include <iostream>
#include <vector>

#include "components.hpp"
#include "ecs/entity.hpp"
#include "world.hpp"

struct Collider {
    float width = 1.0f;
    float height = 1.0f;
};

inline bool overlaps(const Position& pa, const Collider& ca, const Position& pb, const Collider& cb){
    return std::abs(pa.x - pb.x) < (ca.width + cb.width) * 0.5 && std::abs(pa.y - pb.y) < (ca.height + cb.height) * 0.5f;
}

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

inline void collisionSystem(World& world){
    using Entry = std::pair<Entity, std::pair<Position*, Collider*>>;
    std::vector<Entry> collidables;

    world.view<Position, Collider>([&] (Entity e, Position& p, Collider& c){
        collidables.push_back({e, {&p, &c}});
    });

    for (size_t i = 0; i < collidables.size(); ++i){
        for (size_t j = i + 1; j < collidables.size(); ++j){
            auto& [ea, pca] = collidables[i];
            auto& [eb, pcb] = collidables[j];
            if (overlaps(*pca.first, *pca.second, *pcb.first, *pcb.second)){
                world.events().emit(CollisionEvent{ea, eb});
            }
        }
    }
}

inline void healthPrintSystem(World& world) {
    world.forEach<Health>([](Entity e, Health& hp) {
        std::cout << "Entity " << e << " hp: " << hp.hp << "\n";
    });
}

inline void lifetimeSystem(World& world) {
    std::vector<Entity> dead;
    world.forEach<Lifetime>([&](Entity e, Lifetime& lt) {
        lt.framesLeft--;
        if (lt.framesLeft <= 0) dead.push_back(e);
    });
    for (Entity e : dead) world.destroyEntity(e);
}
