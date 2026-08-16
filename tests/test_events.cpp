#include <doctest.h>

#include <vector>

#include "ecs/components.hpp"
#include "ecs/systems.hpp"
#include "ecs/world.hpp"

// ── EventBus ──────────────────────────────────────────────────────────────────

TEST_CASE("emit fires a subscribed handler immediately") {
    World world;
    int fired = 0;
    world.events().subscribe<DamageTaken>([&](const DamageTaken&) { ++fired; });

    world.events().emit(DamageTaken{INVALID_ENTITY, 10.0f});

    CHECK(fired == 1);
}

TEST_CASE("emit with no subscriber is a no-op") {
    World world;
    // Should not throw or crash even with zero subscribers.
    world.events().emit(DamageTaken{INVALID_ENTITY, 5.0f});
}

TEST_CASE("enqueued event is not fired until flush") {
    World world;
    int fired = 0;
    world.events().subscribe<EntityDied>([&](const EntityDied&) { ++fired; });

    world.events().enqueue(EntityDied{INVALID_ENTITY});
    CHECK(fired == 0);

    world.events().flush();
    CHECK(fired == 1);
}

TEST_CASE("World::update flushes queued events after commands") {
    World world;
    std::vector<Entity> died;
    world.events().subscribe<EntityDied>([&](const EntityDied& ev) {
        died.push_back(ev.entity);
    });

    Entity e = world.createEntity();
    world.add<Health>(e, {10.0f, 10.0f});
    world.add<Damage>(e, {10.0f});
    world.addTag<Enemy>(e);

    // damageSystem enqueues EntityDied and defers destroy via CommandBuffer.
    world.registerSystem(damageSystem);
    world.update();

    // After update(), both the command flush and the event flush have run.
    CHECK_FALSE(world.isValid(e));
    REQUIRE(died.size() == 1);
    CHECK(died[0] == e);
}

TEST_CASE("multiple subscribers all receive the same event") {
    World world;
    int a = 0, b = 0;
    world.events().subscribe<DamageTaken>([&](const DamageTaken&) { ++a; });
    world.events().subscribe<DamageTaken>([&](const DamageTaken&) { ++b; });

    world.events().emit(DamageTaken{INVALID_ENTITY, 1.0f});

    CHECK(a == 1);
    CHECK(b == 1);
}

TEST_CASE("events from different types do not cross-fire") {
    World world;
    int damageCount = 0, diedCount = 0;
    world.events().subscribe<DamageTaken>([&](const DamageTaken&) { ++damageCount; });
    world.events().subscribe<EntityDied>([&](const EntityDied&) { ++diedCount; });

    world.events().emit(DamageTaken{INVALID_ENTITY, 5.0f});

    CHECK(damageCount == 1);
    CHECK(diedCount == 0);
}

// ── Lifetime system ───────────────────────────────────────────────────────────

TEST_CASE("entity with Lifetime is destroyed when framesLeft reaches zero") {
    World world;
    world.registerSystem(lifetimeSystem);

    Entity e = world.createEntity();
    world.add<Lifetime>(e, {3});

    world.update(); CHECK(world.isValid(e));
    world.update(); CHECK(world.isValid(e));
    world.update(); CHECK_FALSE(world.isValid(e));
}

TEST_CASE("entity without Lifetime is unaffected by lifetimeSystem") {
    World world;
    world.registerSystem(lifetimeSystem);

    Entity e = world.createEntity();
    world.add<Position>(e, {0.0f, 0.0f});

    for (int i = 0; i < 20; i++) world.update();

    CHECK(world.isValid(e));
}

TEST_CASE("Lifetime of 1 destroys the entity after a single update") {
    World world;
    world.registerSystem(lifetimeSystem);

    Entity e = world.createEntity();
    world.add<Lifetime>(e, {1});

    world.update();
    CHECK_FALSE(world.isValid(e));
}

TEST_CASE("multiple entities with different lifetimes expire independently") {
    World world;
    world.registerSystem(lifetimeSystem);

    Entity short_lived = world.createEntity();
    world.add<Lifetime>(short_lived, {2});

    Entity long_lived = world.createEntity();
    world.add<Lifetime>(long_lived, {5});

    world.update(); world.update();  // frame 2
    CHECK_FALSE(world.isValid(short_lived));
    CHECK(world.isValid(long_lived));

    world.update(); world.update(); world.update();  // frame 5
    CHECK_FALSE(world.isValid(long_lived));
}
