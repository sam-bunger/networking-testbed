#pragma once

#include <memory>
#include "../INetworkEntity.hpp"
#include "./EntityTimeline.hpp"

enum ClientEntityNetworkState {
    LOCAL_ONLY,
    NETWORK_CREATED,
    NETWORK_DELETED
};

template<typename EntityType, class Entity, class Input>
class ClientEntity {
public:
    ClientEntity(INetworkEntity<EntityType, Entity, Input>* entity) 
        : timeline(entity->serializeSize())
        , entity(entity)
        , deleted(false)
        , frameCreated(entity->getController()->getFrameNumber())
        , frameDeleted(-1)
        , networkState(ClientEntityNetworkState::LOCAL_ONLY)
    { } 

    INetworkEntity<EntityType, Entity, Input>* getEntity() const 
    {
        return entity.get();
    }

    bool isMarkedForDeletion()
    {
        return deleted;
    }

    void setDeletionState(bool d)
    {
        if (d) 
        {
            entity->removeFromWorld();
        } 
        else 
        {
            entity->addToWorld();
        }
        deleted = d;
    }

    int getFrameCreated()
    {
        return frameCreated;
    }

    int getFrameDeleted()
    {
        return frameDeleted;
    }

    ClientEntityNetworkState getNetworkState()
    {
        return networkState;
    }

    void resetFrameCreated(int frame)
    {
        frameCreated = frame;
    }

    void resetFrameDeleted(int frame)
    {
        frameDeleted = frame;
    }

    void networkCreated()
    {
        if (networkState == ClientEntityNetworkState::LOCAL_ONLY) 
        {
            networkState = ClientEntityNetworkState::NETWORK_CREATED;
        }
    }

    void networkDeleted()
    {
        networkState = ClientEntityNetworkState::NETWORK_DELETED;
    }

    EntityTimeline<EntityType, Entity, Input> timeline;
private:
    std::shared_ptr<INetworkEntity<EntityType, Entity, Input>> entity;
    bool deleted;
    int frameCreated;
    int frameDeleted;
    ClientEntityNetworkState networkState;
};