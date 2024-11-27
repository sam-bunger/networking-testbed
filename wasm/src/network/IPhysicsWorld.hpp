#pragma once

#include "./INetworkEntity.hpp"
#include "./IEntityController.hpp"

template<typename EntityType, class Entity, class Input>
class IPhysicsWorld
{
    static_assert(std::is_enum_v<EntityType>, "EntityType must be an enum");
    static_assert(std::is_base_of_v<INetworkEntity<EntityType, Entity, Input>, Entity>, "Entity must inherit from INetworkEntity");

public:
    virtual ~IPhysicsWorld() {}

    virtual void physicsStepHook() = 0;
    virtual INetworkEntity<EntityType, Entity, Input>* createEntityHook(int id, EntityType type, EntityController<EntityType, Entity>* controller) = 0;
    virtual Input predictNextInput(const Input &input, int frame) = 0;
};