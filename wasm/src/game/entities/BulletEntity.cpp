#include "./BulletEntity.hpp"
#include "../EntityWorldInterface.hpp"

BulletEntity::BulletEntity(
    int id, 
    GameEntityType type, 
    EntityController<GameEntityType, GameEntity>* controller,
    EntityWorldInterface<GameEntity>* world
) : GameEntity(id, type, controller, world),
    dirX(0), dirY(0)
{}

void BulletEntity::update(const GameInput &input)
{
    x += dirX;
    y += dirY;
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
}

void BulletEntity::deserialize(void* buffer)
{
    GameEntity::deserialize(buffer);

    BulletEntityState* state = (BulletEntityState*)buffer;
    
    dirX = state->dirX;
    dirY = state->dirY;
}