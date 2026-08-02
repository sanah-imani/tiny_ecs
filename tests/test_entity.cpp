#include <doctest.h>

#include "ecs/entity.hpp"

TEST_CASE("entity handles pack an index and a generation") {
    Entity e = makeEntity(42, 7);
    CHECK(entityIndex(e) == 42u);
    CHECK(entityGeneration(e) == 7u);
}

TEST_CASE("entity packing survives the widest legal values") {
    Entity e = makeEntity(ENTITY_INDEX_MASK, 255);
    CHECK(entityIndex(e) == ENTITY_INDEX_MASK);
    CHECK(entityGeneration(e) == 255u);
}

TEST_CASE("distinct generations of one index are distinct handles") {
    CHECK(makeEntity(9, 0) != makeEntity(9, 1));
}
