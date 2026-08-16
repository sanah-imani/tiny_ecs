#include <doctest.h>

#include <vector>

#include "ecs/components.hpp"
#include "ecs/systems.hpp"
#include "ecs/world.hpp"

TEST_CASE("two overlapping entities emit a CollisionEvent") {
    World world;
    world.registerSystem(collisionSystem);

    Entity a = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Collider>(a, {2.0f, 2.0f});

    Entity b = world.createEntity();
    world.add<Position>(b, {1.0f, 0.0f});
    world.add<Collider>(b, {2.0f, 2.0f});

    int fired = 0;
    world.events().subscribe<CollisionEvent>([&](const CollisionEvent&) { ++fired; });

    world.update();

    CHECK(fired == 1);
}

TEST_CASE("two separated entities emit no CollisionEvent") {
    World world;
    world.registerSystem(collisionSystem);

    Entity a = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Collider>(a, {1.0f, 1.0f});

    Entity b = world.createEntity();
    world.add<Position>(b, {10.0f, 0.0f});
    world.add<Collider>(b, {1.0f, 1.0f});

    int fired = 0;
    world.events().subscribe<CollisionEvent>([&](const CollisionEvent&) { ++fired; });

    world.update();

    CHECK(fired == 0);
}

TEST_CASE("three entities: only the overlapping pair fires") {
    World world;
    world.registerSystem(collisionSystem);

    Entity a = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Collider>(a, {2.0f, 2.0f});

    Entity b = world.createEntity();
    world.add<Position>(b, {1.0f, 0.0f});
    world.add<Collider>(b, {2.0f, 2.0f});

    // c is far away — overlaps neither a nor b
    Entity c = world.createEntity();
    world.add<Position>(c, {100.0f, 0.0f});
    world.add<Collider>(c, {2.0f, 2.0f});

    int fired = 0;
    world.events().subscribe<CollisionEvent>([&](const CollisionEvent&) { ++fired; });

    world.update();

    CHECK(fired == 1);
}

TEST_CASE("entity without Collider is not in the broadphase") {
    World world;
    world.registerSystem(collisionSystem);

    Entity a = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Collider>(a, {2.0f, 2.0f});

    // b has a Position but no Collider — should be invisible to collisionSystem
    Entity b = world.createEntity();
    world.add<Position>(b, {0.0f, 0.0f});

    int fired = 0;
    world.events().subscribe<CollisionEvent>([&](const CollisionEvent&) { ++fired; });

    world.update();

    CHECK(fired == 0);
}

TEST_CASE("CollisionEvent carries the correct entity handles") {
    World world;
    world.registerSystem(collisionSystem);

    Entity a = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Collider>(a, {2.0f, 2.0f});

    Entity b = world.createEntity();
    world.add<Position>(b, {1.0f, 0.0f});
    world.add<Collider>(b, {2.0f, 2.0f});

    std::vector<CollisionEvent> events;
    world.events().subscribe<CollisionEvent>([&](const CollisionEvent& ev) {
        events.push_back(ev);
    });

    world.update();

    REQUIRE(events.size() == 1);
    // The pair should be {a, b} in some order
    bool correct = (events[0].a == a && events[0].b == b)
                || (events[0].a == b && events[0].b == a);
    CHECK(correct);
}
