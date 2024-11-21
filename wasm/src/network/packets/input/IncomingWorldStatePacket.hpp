#pragma once

#include "./IncomingNetworkPacket.hpp"
#include "../../INetworkEntity.hpp"
#include "../WorldStatePacket.hpp"
#include <map>

template<typename EntityType, class Entity, class Input>
class IncomingWorldStatePacket : public IncomingNetworkPacket
{
public:
    IncomingWorldStatePacket(const void *p, int size) : IncomingNetworkPacket(p, size)
    {
        WorldStateHeader *header = (WorldStateHeader *)getPacket();
        serverFrame = header->serverFrameNumber;
        controlledEntityCount = header->controlledEntityCount;
        entityCount = header->entityCount;

        char *current = (char *)getPacket() + sizeof(WorldStateHeader);
        for (int i = 0; i < controlledEntityCount; i++)
        {
            WorldStateControlledEntityHeader<Input> *controlledEntityHeader = (WorldStateControlledEntityHeader<Input> *)current;
            int entityId = controlledEntityHeader->id;
            allEntities[entityId] = true;
            controlledEntities[entityId] = true;
            entityHeaders[entityId] = controlledEntityHeader;
            entityData[entityId] = (void*)(current + sizeof(WorldStateControlledEntityHeader<Input>));
            current += sizeof(WorldStateControlledEntityHeader<Input>) + controlledEntityHeader->size;
        }

        for (int i = 0; i < entityCount; i++)
        {
            WorldStateEntityHeader *entityHeader = (WorldStateEntityHeader *)current;
            int entityId = entityHeader->id;
            allEntities[entityId] = true;
            entityHeaders[entityId] = entityHeader;
            entityData[entityId] = (void*)(current + sizeof(WorldStateEntityHeader));
            current += sizeof(WorldStateEntityHeader) + entityHeader->size;
        }
    }

    int getServerFrame() const
    {
        return ((WorldStateHeader *)getPacket())->serverFrameNumber;
    }

    int getControlledEntityCount() const
    {
        return ((WorldStateHeader *)getPacket())->controlledEntityCount;
    }

    int getEntityCount() const
    {
        return ((WorldStateHeader *)getPacket())->entityCount;
    }

    bool isEntityInPacket(int id) const
    {
        return allEntities.find(id) != allEntities.end();
    }

    bool isControlledEntity(int id) const
    {
        return controlledEntities.find(id) != controlledEntities.end();
    }

    WorldStateControlledEntityHeader<Input> *getControlledEntityHeader(int id) const
    {
        return (WorldStateControlledEntityHeader<Input> *)entityHeaders.at(id);
    }

    WorldStateEntityHeader *getEntityHeader(int id) const
    {
        return entityHeaders.at(id);
    }

    void *getEntityData(int id) const
    {
        return entityData.at(id);
    }

private:
    int serverFrame;
    int controlledEntityCount;
    int entityCount;

    std::unordered_map<int, bool> allEntities;
    std::unordered_map<int, bool> controlledEntities;
    std::unordered_map<int, WorldStateEntityHeader *> entityHeaders;
    std::unordered_map<int, void*> entityData;
};