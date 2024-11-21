#pragma once

#include <memory>
#include "../INetworkEntity.hpp"
#include "./EntityTimeline.hpp"

template<typename EntityType, class Entity, class Input>
class ClientEntity {
public:
    ClientEntity(INetworkEntity<EntityType, Entity, Input>* entity) : timeline(entity->serializeSize()), entity(entity)
    { 

    } 

    INetworkEntity<EntityType, Entity, Input>* getEntity() const 
    {
        return entity.get();
    }

    EntityTimeline<EntityType, Entity, Input> timeline;
private:
    std::shared_ptr<INetworkEntity<EntityType, Entity, Input>> entity;
};