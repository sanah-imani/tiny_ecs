#include <doctest.h>

#include <vector>

#include "ecs/components.hpp"
#include "ecs/world.hpp"

TEST_CASE("a fresh entity is valid and the null handle is not") {
    World world;
    Entity e = world.createEntity();
    CHECK(world.isValid(e));
    CHECK_FALSE(world.isValid(INVALID_ENTITY));
}

TEST_CASE("destroying an entity invalidates its handle") {
    World world;
    Entity e = world.createEntity();
    world.destroyEntity(e);
    CHECK_FALSE(world.isValid(e));
}

TEST_CASE("a recycled slot yields a handle that does not collide with the old one") {
    World world;
    Entity first = world.createEntity();
    world.destroyEntity(first);

    Entity second = world.createEntity();
    CHECK(entityIndex(second) == entityIndex(first));
    CHECK(second != first);
    CHECK(world.isValid(second));
    CHECK_FALSE(world.isValid(first));
}

TEST_CASE("destroying an entity strips every component it owned") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {1.0f, 2.0f});
    world.add<Health>(e, {50.0f, 100.0f});
    world.addTag<Enemy>(e);

    world.destroyEntity(e);
    Entity reused = world.createEntity();

    CHECK_FALSE(world.has<Position>(reused));
    CHECK_FALSE(world.has<Health>(reused));
    CHECK_FALSE(world.hasTag<Enemy>(reused));
}

TEST_CASE("components round-trip through add and get") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {3.0f, 4.0f});

    Position* pos = world.get<Position>(e);
    REQUIRE(pos != nullptr);
    CHECK(pos->x == doctest::Approx(3.0f));
    CHECK(pos->y == doctest::Approx(4.0f));
}

TEST_CASE("get returns null for a component the entity does not have") {
    World world;
    Entity e = world.createEntity();
    CHECK(world.get<Velocity>(e) == nullptr);
}

TEST_CASE("remove drops a single component and leaves the rest") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {1.0f, 1.0f});
    world.add<Velocity>(e, {2.0f, 2.0f});

    world.remove<Position>(e);
    CHECK_FALSE(world.has<Position>(e));
    CHECK(world.has<Velocity>(e));
}

TEST_CASE("adding to a stale handle is a no-op") {
    World world;
    Entity e = world.createEntity();
    world.destroyEntity(e);
    world.add<Position>(e, {1.0f, 1.0f});

    Entity reused = world.createEntity();
    CHECK_FALSE(world.has<Position>(reused));
}

TEST_CASE("view only visits entities holding every requested component") {
    World world;

    Entity both = world.createEntity();
    world.add<Position>(both, {0.0f, 0.0f});
    world.add<Velocity>(both, {1.0f, 1.0f});

    Entity positionOnly = world.createEntity();
    world.add<Position>(positionOnly, {0.0f, 0.0f});

    std::vector<Entity> visited;
    world.view<Position, Velocity>([&](Entity e, Position&, Velocity&) { visited.push_back(e); });

    REQUIRE(visited.size() == 1);
    CHECK(visited[0] == both);
}

TEST_CASE("view hands out references that write through to storage") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {0.0f, 0.0f});
    world.add<Velocity>(e, {1.0f, 2.0f});

    world.view<Position, Velocity>([](Entity, Position& pos, Velocity& vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });

    CHECK(world.get<Position>(e)->x == doctest::Approx(1.0f));
    CHECK(world.get<Position>(e)->y == doctest::Approx(2.0f));
}

TEST_CASE("a single-component view visits every holder of that component") {
    World world;
    Entity a = world.createEntity();
    Entity b = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Position>(b, {0.0f, 0.0f});
    world.createEntity();

    int visits = 0;
    world.view<Position>([&](Entity, Position&) { ++visits; });
    CHECK(visits == 2);
}

TEST_CASE("a three-component view requires all three") {
    World world;

    Entity all = world.createEntity();
    world.add<Position>(all, {0.0f, 0.0f});
    world.add<Velocity>(all, {0.0f, 0.0f});
    world.add<Health>(all, {10.0f, 10.0f});

    Entity two = world.createEntity();
    world.add<Position>(two, {0.0f, 0.0f});
    world.add<Velocity>(two, {0.0f, 0.0f});

    std::vector<Entity> visited;
    world.view<Position, Velocity, Health>(
        [&](Entity e, Position&, Velocity&, Health&) { visited.push_back(e); });

    REQUIRE(visited.size() == 1);
    CHECK(visited[0] == all);
}

TEST_CASE("view result does not depend on which component pool is smallest") {
    World world;
    Entity tagged = INVALID_ENTITY;

    for (int i = 0; i < 50; ++i) {
        Entity e = world.createEntity();
        world.add<Position>(e, {0.0f, 0.0f});
        if (i == 25) {
            world.addTag<Player>(e);
            tagged = e;
        }
    }

    std::vector<Entity> forward;
    world.view<Position, Player>([&](Entity e, Position&, Player&) { forward.push_back(e); });

    std::vector<Entity> reversed;
    world.view<Player, Position>([&](Entity e, Player&, Position&) { reversed.push_back(e); });

    CHECK(forward == std::vector<Entity>{tagged});
    CHECK(reversed == std::vector<Entity>{tagged});
}

TEST_CASE("registered systems run in order on every update") {
    World world;
    std::vector<int> order;

    world.registerSystem([&](World&) { order.push_back(1); });
    world.registerSystem([&](World&) { order.push_back(2); });

    world.update();
    world.update();

    CHECK(order == std::vector<int>{1, 2, 1, 2});
}
