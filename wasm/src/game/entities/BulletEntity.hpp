#pragma once

#include "../GameEntity.hpp"

#pragma pack(1)
struct BulletEntityState : GameEntityState
{
    float dirX;
    float dirY;
};
#pragma pack()

class BulletEntity : public GameEntity
{
public:
    BulletEntity(
        int id, 
        GameEntityType type, 
        EntityController<GameEntityType, GameEntity>* controller,
        EntityWorldInterface<GameEntity>* world
    );

    virtual void update(const GameInput &input) override;
    virtual int serializeSize() override;
    virtual void serialize(void* buffer) override;
    virtual void deserialize(void* buffer) override;

    void setDirection(float x, float y);

private:
    float dirX;
    float dirY;
};