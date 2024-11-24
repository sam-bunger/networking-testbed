#pragma once

#include "../GameEntity.hpp"

#pragma pack(1)
struct PlayerEntityState : GameEntityState
{
    float radius;
    char fireCooldown;
};
#pragma pack()

class PlayerEntity : public GameEntity
{
public:
    PlayerEntity(
        int id, 
        GameEntityType type, 
        EntityController<GameEntityType, GameEntity>* controller,
        EntityWorldInterface<GameEntity>* world
    );

    virtual void update(const GameInput &input) override;
    virtual int serializeSize() override;
    virtual void serialize(void* buffer) override;
    virtual void deserialize(void* buffer) override;

    float getRadius() { return radius; }

    float radius;
    char fireCooldown;

private:
    void updateFiring(const GameInput &input);
};