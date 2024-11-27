#pragma once

#include <memory>
#include <string>
#include "../INetworkEntity.hpp"
#include "./ServerEntityHistory.hpp"

template<typename EntityType, class Entity, class Input>
class ServerEntity {
public:
    ServerEntity(INetworkEntity<EntityType, Entity, Input>* entityPtr) 
        : entity(entityPtr)
        , readyForDeletion(false)
        , history(entity)
    { } 

    INetworkEntity<EntityType, Entity, Input>* getEntity() const 
    {
        return entity.get();
    }

    bool isEnabled() const 
    {
        return !readyForDeletion;
    }

    bool isReadyForDeletion() const 
    {
        return readyForDeletion && history.isHistoryEmpty();
    }

    void setReadyForDeletion(bool ready) 
    {
        readyForDeletion = ready;
    }

    ServerEntityHistroy<EntityType, Entity, Input>& getHistory()
    {
        return history;
    }
private:
    std::shared_ptr<INetworkEntity<EntityType, Entity, Input>> entity;
    bool readyForDeletion;
    ServerEntityHistroy<EntityType, Entity, Input> history;
};