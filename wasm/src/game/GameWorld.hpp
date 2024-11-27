#pragma once

#include <map>
#include "../network/IPhysicsWorld.hpp"
#include "./EntityWorldInterface.hpp"
#include "./GameInput.hpp"
#include "./GameEntity.hpp"

class GameWorld : public IPhysicsWorld<GameEntityType, GameEntity, GameInput>,
                  public EntityWorldInterface<GameEntity>
{
public:
    GameWorld();
    virtual ~GameWorld();

    virtual void physicsStepHook() override;
    virtual INetworkEntity<GameEntityType, GameEntity, GameInput>* createEntityHook(int id, GameEntityType type, EntityController<GameEntityType, GameEntity>* controller) override;
    virtual GameInput predictNextInput(const GameInput &input, int frame) override;

    virtual void addEntityToWorld(GameEntity* entity) override;
    virtual void removeEntityFromWorld(GameEntity* entity) override;

private:
    std::map<int, GameEntity*> entities;
};

