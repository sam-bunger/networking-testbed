#include "./BulletEntity.hpp"
#include "../EntityWorldInterface.hpp"

BulletEntity::BulletEntity(
    int id, 
    GameEntityType type, 
    EntityController<GameEntityType, GameEntity>* controller,
    EntityWorldInterface<GameEntity>* world
) : GameEntity(id, type, controller, world),
    dirX(0), dirY(0), life(30)
{}

void BulletEntity::update(const GameInput &input)
{
    if (life > 0) {
        --life;
    } else if (life == 0) {
        getController()->destroyEntity(this->getId());
        life = -1;
    }

    vx = dirX * 6.5f;
    vy = dirY * 6.5f;
}

int BulletEntity::serializeSize()
{
    return sizeof(BulletEntityState);
}

void BulletEntity::serialize(void* buffer)
{
    GameEntity::serialize(buffer);

    BulletEntityState* state = (BulletEntityState*)buffer;
    
    state->dirX = dirX;
    state->dirY = dirY;
    state->life = life;
}

void BulletEntity::deserialize(void* buffer)
{
    GameEntity::deserialize(buffer);

    BulletEntityState* state = (BulletEntityState*)buffer;
    
    dirX = state->dirX;
    dirY = state->dirY;
    life = state->life;
}

void BulletEntity::setDirection(float x, float y)
{
    // Calculate magnitude
    float magnitude = sqrt(x * x + y * y);
    
    // Normalize the direction vector (avoid division by zero)
    if (magnitude > 0) {
        dirX = x / magnitude;
        dirY = y / magnitude;
    } else {
        dirX = 0;
        dirY = 0;
    }
}
