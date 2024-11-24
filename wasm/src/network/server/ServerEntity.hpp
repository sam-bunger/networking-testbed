#pragma once

#include <memory>
#include <string>
#include "../INetworkEntity.hpp"

template<typename EntityType, class Entity, class Input>
class ServerEntity {
public:
    ServerEntity(INetworkEntity<EntityType, Entity, Input>* entity) 
        : entity(entity)
        , updated(true)
        , lastHash(0)
        , readyForDeletion(false)
    { } 

    INetworkEntity<EntityType, Entity, Input>* getEntity() const {
        return entity.get();
    }

    bool needsUpdate() const {
        return updated;
    }

    void calculateIfUpdated() {
        int size = entity->serializeSize();
        std::vector<char> snapshot(size);
        entity->serialize(snapshot.data());
        
        std::string currentState(snapshot.data(), size);
        size_t hash = std::hash<std::string>{}(currentState);
        
        if (hash != lastHash) {
            lastHash = hash;
            updated = true;
        }
    }

    bool isReadyForDeletion() const {
        return readyForDeletion;
    }

    void setReadyForDeletion(bool ready) {
        readyForDeletion = ready;
    }

private:
    std::shared_ptr<INetworkEntity<EntityType, Entity, Input>> entity;
    bool updated;
    size_t lastHash;
    bool readyForDeletion;
};