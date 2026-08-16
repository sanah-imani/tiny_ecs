#include <doctest.h>
#include <cstdio>
#include <string>

#include "ecs/components.hpp"
#include "ecs/scene_serializer.hpp"
#include "ecs/world.hpp"

using json = nlohmann::json;

static SceneSerializer makeSerializer() {
    SceneSerializer s;

    s.registerComponent<Position>("Position",
        [](const Position& p) -> json { return {{"x", p.x}, {"y", p.y}}; },
        [](const json& j) -> Position { return {j["x"].get<float>(), j["y"].get<float>()}; }
    );
    s.registerComponent<Velocity>("Velocity",
        [](const Velocity& v) -> json { return {{"vx", v.vx}, {"vy", v.vy}}; },
        [](const json& j) -> Velocity { return {j["vx"].get<float>(), j["vy"].get<float>()}; }
    );
    s.registerComponent<Health>("Health",
        [](const Health& h) -> json { return {{"hp", h.hp}, {"maxHp", h.maxHp}}; },
        [](const json& j) -> Health { return {j["hp"].get<float>(), j["maxHp"].get<float>()}; }
    );
    s.registerComponent<Player>("Player",
        [](const Player&) -> json { return {}; },
        [](const json&)   -> Player { return {}; }
    );
    s.registerComponent<Enemy>("Enemy",
        [](const Enemy&) -> json { return {}; },
        [](const json&)  -> Enemy { return {}; }
    );

    return s;
}

static const std::string TMP = "/tmp/tiny_ecs_test_scene.json";

TEST_CASE("Position and Velocity round-trip through save/load") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {3.0f, 7.0f});
    world.add<Velocity>(e, {1.5f, -2.0f});

    auto s = makeSerializer();
    s.save(world, TMP);

    World loaded;
    s.load(loaded, TMP);

    int found = 0;
    loaded.view<Position, Velocity>([&](Entity, Position& p, Velocity& v) {
        ++found;
        CHECK(p.x  == doctest::Approx(3.0f));
        CHECK(p.y  == doctest::Approx(7.0f));
        CHECK(v.vx == doctest::Approx(1.5f));
        CHECK(v.vy == doctest::Approx(-2.0f));
    });
    CHECK(found == 1);

    std::remove(TMP.c_str());
}

TEST_CASE("entity count is preserved after load") {
    World world;
    for (int i = 0; i < 5; i++) {
        Entity e = world.createEntity();
        world.add<Position>(e, {(float)i, 0.0f});
    }

    auto s = makeSerializer();
    s.save(world, TMP);

    World loaded;
    s.load(loaded, TMP);

    int count = 0;
    loaded.forEach<Position>([&](Entity, Position&) { ++count; });
    CHECK(count == 5);

    std::remove(TMP.c_str());
}

TEST_CASE("tags (zero-size structs) round-trip") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {0.0f, 0.0f});
    world.addTag<Player>(e);

    auto s = makeSerializer();
    s.save(world, TMP);

    World loaded;
    s.load(loaded, TMP);

    int tagged = 0;
    loaded.forEach<Player>([&](Entity, Player&) { ++tagged; });
    CHECK(tagged == 1);

    std::remove(TMP.c_str());
}

TEST_CASE("load into a fresh world produces only the loaded entities") {
    World world;
    Entity e = world.createEntity();
    world.add<Health>(e, {50.0f, 100.0f});

    auto s = makeSerializer();
    s.save(world, TMP);

    World loaded;
    s.load(loaded, TMP);

    int count = 0;
    loaded.forEach<Health>([&](Entity, Health&) { ++count; });
    CHECK(count == 1);

    std::remove(TMP.c_str());
}

TEST_CASE("unregistered components in the file are silently skipped") {
    World world;
    Entity e = world.createEntity();
    world.add<Position>(e, {1.0f, 2.0f});
    world.add<Health>(e, {80.0f, 100.0f});

    auto s = makeSerializer();
    s.save(world, TMP);

    SceneSerializer partial;
    partial.registerComponent<Position>("Position",
        [](const Position& p) -> json { return {{"x", p.x}, {"y", p.y}}; },
        [](const json& j) -> Position { return {j["x"].get<float>(), j["y"].get<float>()}; }
    );

    World loaded;
    partial.load(loaded, TMP);

    int found = 0;
    loaded.forEach<Position>([&](Entity, Position&) { ++found; });
    CHECK(found == 1);

    std::remove(TMP.c_str());
}

TEST_CASE("loading a missing file throws") {
    auto s = makeSerializer();
    World world;
    CHECK_THROWS(s.load(world, "/tmp/does_not_exist_tiny_ecs.json"));
}
