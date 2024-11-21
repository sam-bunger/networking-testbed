#include "./PlayerEntity.hpp"
#include "../EntityWorldInterface.hpp"
#include <iostream>
#include <cmath>

PlayerEntity::PlayerEntity(
    int id, 
    GameEntityType type, 
    EntityController<GameEntityType, GameEntity>* controller,
    EntityWorldInterface<GameEntity>* world
) : GameEntity(id, type, controller, world),
    radius(10)
{}



void PlayerEntity::update(const GameInput &input)
{
    float dx = (float)input.leftRight;
    float dy = (float)input.upDown;
    
    // Normalize only if we have input (avoid division by zero)
    if (dx != 0.0f || dy != 0.0f) {
        float length = sqrt(dx * dx + dy * dy);
        dx /= length;
        dy /= length;
        
        // Apply speed and add to velocity
        const float speed = 0.3f;
        vx += dx * speed;
        vy += dy * speed;
    }

    vx *= 0.9f;
    vy *= 0.9f;
}

int PlayerEntity::serializeSize()
{
    return sizeof(PlayerEntityState);
}

void PlayerEntity::serialize(void* buffer)
{
    GameEntity::serialize(buffer);

    PlayerEntityState* state = (PlayerEntityState*)buffer;
    
    state->radius = radius;
}

void PlayerEntity::deserialize(void* buffer)
{
    GameEntity::deserialize(buffer);

    PlayerEntityState* state = (PlayerEntityState*)buffer;
    
    radius = state->radius;
}