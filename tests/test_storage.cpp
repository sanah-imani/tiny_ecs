#include <doctest.h>

#include <vector>

#include "ecs/component_storage.hpp"
#include "ecs/components.hpp"

TEST_CASE("storage keeps distinct component types in distinct pools") {
    ComponentStorage storage;
    storage.add<Position>(1, {1.0f, 1.0f});
    storage.add<Velocity>(1, {2.0f, 2.0f});

    CHECK(storage.get<Position>(1)->x == doctest::Approx(1.0f));
    CHECK(storage.get<Velocity>(1)->vx == doctest::Approx(2.0f));
}

TEST_CASE("has and get agree about absence") {
    ComponentStorage storage;
    CHECK_FALSE(storage.has<Position>(3));
    CHECK(storage.get<Position>(3) == nullptr);
}

TEST_CASE("adding twice overwrites rather than duplicating") {
    ComponentStorage storage;
    storage.add<Position>(1, {1.0f, 1.0f});
    storage.add<Position>(1, {9.0f, 9.0f});

    CHECK(storage.get<Position>(1)->x == doctest::Approx(9.0f));

    int visits = 0;
    storage.forEach<Position>([&](uint32_t, Position&) { ++visits; });
    CHECK(visits == 1);
}

TEST_CASE("removeAll clears the slot across every pool it appears in") {
    ComponentStorage storage;
    storage.add<Position>(1, {1.0f, 1.0f});
    storage.add<Velocity>(1, {1.0f, 1.0f});
    storage.add<Position>(2, {2.0f, 2.0f});

    storage.removeAll(1);

    CHECK_FALSE(storage.has<Position>(1));
    CHECK_FALSE(storage.has<Velocity>(1));
    CHECK(storage.has<Position>(2));
}

TEST_CASE("forEach visits every live component exactly once") {
    ComponentStorage storage;
    storage.add<Position>(1, {1.0f, 0.0f});
    storage.add<Position>(2, {2.0f, 0.0f});
    storage.add<Position>(3, {3.0f, 0.0f});
    storage.remove<Position>(2);

    float sum = 0.0f;
    int visits = 0;
    storage.forEach<Position>([&](uint32_t, Position& p) {
        sum += p.x;
        ++visits;
    });

    CHECK(visits == 2);
    CHECK(sum == doctest::Approx(4.0f));
}
