#include <doctest.h>

#include "ecs/components.hpp"
#include "ecs/world.hpp"

TEST_CASE("a component added during a view appears only after the flush") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {0.0f, 0.0f});

    CommandBuffer& cmd = world.commands();
    world.view<Position>([&](Entity id, Position&) { cmd.add<Velocity>(id, {1.0f, 2.0f}); });

    CHECK_FALSE(world.has<Velocity>(e));

    world.flushCommands();

    REQUIRE(world.get<Velocity>(e) != nullptr);
    CHECK(world.get<Velocity>(e)->vx == doctest::Approx(1.0f));
    CHECK(world.get<Velocity>(e)->vy == doctest::Approx(2.0f));
    CHECK(world.commands().empty());
}

TEST_CASE("an entity destroyed during a view survives until the flush") {
    World world;
    Entity a = world.createEntity();
    Entity b = world.createEntity();
    world.add<Position>(a, {0.0f, 0.0f});
    world.add<Position>(b, {0.0f, 0.0f});

    CommandBuffer& cmd = world.commands();
    int visits = 0;
    world.view<Position>([&](Entity id, Position&) {
        ++visits;
        cmd.destroy(id);
    });

    CHECK(visits == 2);
    CHECK(world.isValid(a));
    CHECK(world.isValid(b));

    world.flushCommands();

    CHECK_FALSE(world.isValid(a));
    CHECK_FALSE(world.isValid(b));
    CHECK_FALSE(world.has<Position>(a));
}

TEST_CASE("destroying the same entity twice in one flush is harmless") {
    World world;
    Entity e = world.createEntity();
    world.add<Health>(e, {10.0f, 10.0f});

    CommandBuffer& cmd = world.commands();
    cmd.destroy(e);
    cmd.destroy(e);
    world.flushCommands();

    CHECK_FALSE(world.isValid(e));

    Entity reused = world.createEntity();
    CHECK(entityIndex(reused) == entityIndex(e));
    CHECK(reused != e);
    CHECK(world.isValid(reused));
}

TEST_CASE("commands recorded against a stale handle are dropped") {
    World world;
    Entity doomed = world.createEntity();

    CommandBuffer& cmd = world.commands();
    cmd.destroy(doomed);
    cmd.add<Position>(doomed, {5.0f, 5.0f});
    cmd.addTag<Enemy>(doomed);
    world.flushCommands();

    Entity reused = world.createEntity();
    REQUIRE(entityIndex(reused) == entityIndex(doomed));
    CHECK_FALSE(world.has<Position>(reused));
    CHECK_FALSE(world.hasTag<Enemy>(reused));
}

TEST_CASE("a deferred remove leaves a recycled slot alone") {
    World world;
    Entity first = world.createEntity();
    world.add<Position>(first, {1.0f, 1.0f});
    world.destroyEntity(first);

    Entity reused = world.createEntity();
    world.add<Position>(reused, {2.0f, 2.0f});

    world.commands().remove<Position>(first);
    world.flushCommands();

    REQUIRE(world.get<Position>(reused) != nullptr);
    CHECK(world.get<Position>(reused)->x == doctest::Approx(2.0f));
}

TEST_CASE("commands run in the order they were recorded") {
    World world;
    Entity e = world.createEntity();

    CommandBuffer& cmd = world.commands();
    cmd.add<Position>(e, {1.0f, 0.0f});
    cmd.add<Position>(e, {2.0f, 0.0f});
    world.flushCommands();

    REQUIRE(world.get<Position>(e) != nullptr);
    CHECK(world.get<Position>(e)->x == doctest::Approx(2.0f));
}

TEST_CASE("a command recorded during the flush runs in the same flush") {
    World world;
    Entity e = world.createEntity();

    world.commands().record([e](World& w) { w.commands().add<Health>(e, {7.0f, 7.0f}); });
    world.flushCommands();

    REQUIRE(world.get<Health>(e) != nullptr);
    CHECK(world.get<Health>(e)->hp == doctest::Approx(7.0f));
    CHECK(world.commands().empty());
}

TEST_CASE("update flushes the frame's commands once, after every system") {
    World world;
    Entity e = world.createEntity();
    world.add<Health>(e, {10.0f, 10.0f});
    world.add<Damage>(e, {10.0f});
    world.addTag<Enemy>(e);

    bool laterSystemSawIt = false;
    world.registerSystem([](World& w) {
        w.view<Health, Damage>([&](Entity id, Health& hp, Damage& dmg) {
            hp.hp -= dmg.amount;
            if (hp.hp <= 0.0f) w.commands().destroy(id);
        });
    });
    world.registerSystem([&](World& w) { laterSystemSawIt = w.isValid(e); });

    world.update();

    CHECK(laterSystemSawIt);
    CHECK_FALSE(world.isValid(e));
    CHECK(world.commands().empty());
}
