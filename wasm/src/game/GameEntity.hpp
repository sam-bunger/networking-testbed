#pragma once

#include "../network/INetworkEntity.hpp"
#include "./GameInput.hpp"
#include "./EntityWorldInterface.hpp"

enum GameEntityType {
    PLAYER,
    BULLET,
};

#pragma pack(1)
struct GameEntityState 
{
    float x;
    float y;
    float vx;
    float vy;
};
#pragma pack()

class GameEntity : public INetworkEntity<GameEntityType, GameEntity, GameInput>
{
public:
    GameEntity(
        int id, 
        GameEntityType type, 
        EntityController<GameEntityType, GameEntity>* controller, 
        EntityWorldInterface<GameEntity>* world
    );
    virtual ~GameEntity();

    virtual int serializeSize() override;
    virtual void serialize(void* buffer) override;
    virtual void deserialize(void* buffer) override;

    float getX();
    float getY();
    float getVx();
    float getVy();

    float x, y;
    float vx, vy;

private: 
    EntityWorldInterface<GameEntity>* world;
};