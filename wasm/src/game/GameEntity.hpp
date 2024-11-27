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

    bool operator!=(const GameEntityState& other) const
    {
        return x != other.x || y != other.y || vx != other.vx || vy != other.vy;
    }

    GameEntityState operator+(const GameEntityState& other) const
    {
        return GameEntityState{
            x + other.x,
            y + other.y,
            vx + other.vx,
            vy + other.vy
        };
    }

    GameEntityState operator-(const GameEntityState& other) const
    {
        return GameEntityState{
            x - other.x,
            y - other.y,
            vx - other.vx,
            vy - other.vy
        };
    }
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

    virtual void addToWorld() override;
    virtual void removeFromWorld() override;

    virtual int serializeSize() override;
    virtual void serialize(void* buffer) override;
    virtual void deserialize(void* buffer) override;
    virtual void reset() override;

    virtual std::shared_ptr<DeltaState> createTypedDiff(void *oldState, void *newState) override;
    virtual void applyTypedDiff(void *oldState, DeltaState *deltaState, void *resultBuffer) override;

    float getX();
    float getY();
    float getVx();
    float getVy();

    float x, y;
    float vx, vy;

private: 
    EntityWorldInterface<GameEntity>* world;
};