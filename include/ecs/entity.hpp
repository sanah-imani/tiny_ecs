#pragma once
#include <cstdint>

using Entity = uint32_t;

constexpr Entity INVALID_ENTITY = 0;

constexpr uint32_t ENTITY_INDEX_BITS = 24;
constexpr uint32_t ENTITY_GEN_BITS = 8;
constexpr uint32_t ENTITY_INDEX_MASK = (1 << ENTITY_INDEX_BITS) - 1;
constexpr uint32_t ENTITY_GEN_MASK = (1 << ENTITY_GEN_BITS) - 1;

inline uint32_t entityIndex(Entity e) {return e & ENTITY_INDEX_MASK;}
inline uint8_t entityGeneration(Entity e) {return (e >> ENTITY_INDEX_BITS) & ENTITY_GEN_MASK;}
inline Entity makeEntity(uint32_t index, uint8_t gen){
    return (static_cast<uint32_t>(gen) << ENTITY_INDEX_BITS) | (index & ENTITY_INDEX_MASK);
}


