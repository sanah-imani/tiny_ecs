#pragma once 

#include "ecs/entity.hpp"
struct Position {
	float x = 0.0f;
	float y = 0.0f;
};

struct Velocity {
	float vx = 0.0f;
	float vy = 0.0f;
};

struct Health{
	float hp = 100.0f;
	float maxHp = 100.0f;
};

struct Damage{
	float amount = 10.0f;
};

// Tags — zero-size marker types, no data
struct Enemy {};
struct Player {};

struct EntityDied {Entity entity;};
struct DamageTaken {Entity entity; float damageAmount;};
struct CollisionEvent {Entity a; Entity b;};