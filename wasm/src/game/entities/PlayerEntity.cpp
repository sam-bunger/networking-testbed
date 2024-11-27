#include "./PlayerEntity.hpp"
#include "../EntityWorldInterface.hpp"
#include "./BulletEntity.hpp"
#include <iostream>
#include <cmath>

PlayerEntity::PlayerEntity(
    int id, 
    GameEntityType type, 
    EntityController<GameEntityType, GameEntity>* controller,
    EntityWorldInterface<GameEntity>* world
) : GameEntity(id, type, controller, world),
    radius(14),
    fireCooldown(0)
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
        const float speed = 0.6f;
        vx += dx * speed;
        vy += dy * speed;
    }

    vx *= 0.9f;
    vy *= 0.9f;

    if (std::abs(vx) < 0.01f) vx = 0;
    if (std::abs(vy) < 0.01f) vy = 0;

    updateFiring(input);
}

int PlayerEntity::serializeSize()
{
    return sizeof(PlayerEntityState);
}

void PlayerEntity::serialize(void* buffer)
{
    PlayerEntityState* state = (PlayerEntityState*)buffer;

    GameEntity::serialize(&state->entity);
    
    state->radius = radius;
    state->fireCooldown = fireCooldown;
}

void PlayerEntity::deserialize(void* buffer)
{
    PlayerEntityState* state = (PlayerEntityState*)buffer;

    GameEntity::deserialize(&state->entity);
    
    radius = state->radius;
    fireCooldown = state->fireCooldown;
}

void PlayerEntity::reset() 
{
    radius = 14;
    fireCooldown = 0;
}

std::shared_ptr<DeltaState> PlayerEntity::createTypedDiff(void *oldState, void *newState) 
{
    return createDiff<PlayerEntityState>((PlayerEntityState*)oldState, (PlayerEntityState*)newState);
}

void PlayerEntity::applyTypedDiff(void *oldState, DeltaState *deltaState, void *resultBuffer) 
{
    return applyDiff<PlayerEntityState>((PlayerEntityState*)oldState, deltaState, (PlayerEntityState*)resultBuffer);
}

void PlayerEntity::updateFiring(const GameInput &input)
{
    if (input.fire && fireCooldown == 0) {
        fireCooldown = 10;
        int leftRight = input.leftRight;
        int upDown = input.upDown;

        if (leftRight == 0 && upDown == 0) {
            upDown = -1;
        }

        BulletEntity *bullet = (BulletEntity *)getController()->createEntity(GameEntityType::BULLET);
        bullet->setDirection(leftRight, upDown);

        //Place position in front of player
        bullet->x = x + (radius + 5) * leftRight;
        bullet->y = y + (radius + 5) * upDown;
    }
    else if (fireCooldown > 0) 
    {
        fireCooldown--;
    }
}
