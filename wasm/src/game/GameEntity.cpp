#include "./GameEntity.hpp"
#include "./EntityWorldInterface.hpp"
#include <iostream>

GameEntity::GameEntity(
    int id, 
    GameEntityType type, 
    EntityController<GameEntityType, GameEntity>* controller,
    EntityWorldInterface<GameEntity>* world
) : INetworkEntity<GameEntityType, GameEntity, GameInput>(id, type, controller)
{
    x = 0;
    y = 0;
    vx = 0;
    vy = 0;

    this->world = world;
    addToWorld();
}

GameEntity::~GameEntity()
{
    removeFromWorld();
}

void GameEntity::addToWorld() 
{
    this->world->addEntityToWorld(this);
}

void GameEntity::removeFromWorld() 
{
    this->world->removeEntityFromWorld(this);
}


int GameEntity::serializeSize() 
{
    return sizeof(GameEntityState);
}

float GameEntity::getX()
{
    return x;
}

float GameEntity::getY()
{
    return y;
}

float GameEntity::getVx()
{
    return vx;
}   

float GameEntity::getVy()
{
    return vy;
}

void GameEntity::serialize(void* buffer) 
{
    GameEntityState* state = (GameEntityState*)buffer;
    state->x = x;
    state->y = y;
    state->vx = vx;
    state->vy = vy;
}

void GameEntity::deserialize(void* buffer) 
{
    GameEntityState* state = (GameEntityState*)buffer;
    x = state->x;
    y = state->y;
    vx = state->vx;
    vy = state->vy;
}

void GameEntity::reset() 
{
    x = 0;
    y = 0;
    vx = 0;
    vy = 0;
}

std::shared_ptr<DeltaState> GameEntity::createTypedDiff(void *oldState, void *newState) 
{
    return createDiff<GameEntityState>((GameEntityState*)oldState, (GameEntityState*)newState);
}

void GameEntity::applyTypedDiff(void *oldState, DeltaState *deltaState, void *resultBuffer) 
{
    return applyDiff<GameEntityState>((GameEntityState*)oldState, deltaState, (GameEntityState*)resultBuffer);
}
