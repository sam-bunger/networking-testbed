#pragma once

#include "./IEntityController.hpp"

template<typename EntityType, class Entity, class Input> 
class INetworkEntity
{
public:
    INetworkEntity(int id, EntityType type, EntityController<EntityType, Entity>* controller) {
        this->id = id;
        this->type = type;
        this->controller = (EntityController<EntityType, Entity>*)controller;
    }

    virtual ~INetworkEntity() {}

    int getId() { return id; }
    EntityType getType() { return type; }
    EntityController<EntityType, Entity>* getController() { return controller; }

    virtual void addToWorld() = 0;
    virtual void removeFromWorld() = 0;

    virtual void update(const Input &input) = 0;
    virtual int serializeSize() = 0;
    virtual void serialize(void* buffer) = 0;
    virtual void deserialize(void* buffer) = 0;

private:
    int id;
    EntityType type;
    EntityController<EntityType, Entity>* controller;
};