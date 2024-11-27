#pragma once

#include <memory>

#include "../INetworkEntity.hpp"
#include "../packets/WorldStatePacket.hpp"

class ServerEntityState
{
public:
    ServerEntityState(int dataSize) : size(dataSize), data(std::make_unique<std::byte[]>(dataSize))
    {}

    int size;
    std::unique_ptr<std::byte[]> data;
};

template<typename EntityType, class Entity, class Input>
class ServerEntityHistroy
{
public:
    ServerEntityHistroy(std::shared_ptr<INetworkEntity<EntityType, Entity, Input>> entity) 
        : entity(entity)
    { } 

    
    void serializeCurrentState()
    {
        int dataSize = entity->serializeSize();
        int frameNumber = entity->getController()->getFrameNumber();

        if (history.find(frameNumber) == history.end()) 
        {
            history[frameNumber] = std::make_shared<ServerEntityState>(dataSize);
        }
        entity->serialize(history[frameNumber]->data.get());
    }

    WorldStateEntityPacketType getPacketType(int fromFrameNumber, int toFrameNumber) const
    {
        if (history.find(fromFrameNumber) == history.end() && history.find(toFrameNumber) == history.end()) 
        {
            return WorldStateEntityPacketType::ENTITY_SKIPPED;
        }
        else if (history.find(fromFrameNumber) == history.end() && history.find(toFrameNumber) != history.end())
        {
            return WorldStateEntityPacketType::ENTITY_FULL_RELOAD;
        }
        else if (history.find(fromFrameNumber) != history.end() && history.find(toFrameNumber) == history.end())
        {
            return WorldStateEntityPacketType::ENTITY_DELETED;
        }


        return WorldStateEntityPacketType::ENTITY_DELTA;
    }

    std::shared_ptr<DeltaState> getDiffState(int fromFrameNumber, int toFrameNumber)
    {
        auto cacheIt = diffCache.find({fromFrameNumber, toFrameNumber});
        if (cacheIt != diffCache.end()) {
            return cacheIt->second;
        }

        auto fromState = history.find(fromFrameNumber);
        auto toState = history.find(toFrameNumber);

        if (fromState == history.end() || toState == history.end()) {
            return nullptr;
        }

        auto result = entity->createTypedDiff(fromState->second->data.get(), toState->second->data.get());
        diffCache.emplace(std::make_pair(std::make_pair(fromFrameNumber, toFrameNumber), result));
        return result;
    }

    void clearHistoryUntil(int frameNumber)
    {
        history.erase(history.begin(), history.lower_bound(frameNumber));
        
        auto cacheIt = diffCache.begin();
        while (cacheIt != diffCache.end()) {
            if (cacheIt->first.first < frameNumber || cacheIt->first.second < frameNumber) {
                cacheIt = diffCache.erase(cacheIt);
            } else {
                ++cacheIt;
            }
        }
    }

    bool isHistoryEmpty() const
    {
        return history.empty();
    }

private:
    std::shared_ptr<INetworkEntity<EntityType, Entity, Input>> entity;
    std::map<int, std::shared_ptr<ServerEntityState>> history;
    std::map<std::pair<int, int>, std::shared_ptr<DeltaState>> diffCache;
};