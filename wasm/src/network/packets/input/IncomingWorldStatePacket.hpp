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

    int getFinalServerFrame() const
    {
        return ((WorldStateHeader *)getPacket())->finalServerFrame;
    }

    int getInitialServerFrame() const
    {
        return ((WorldStateHeader *)getPacket())->initialServerFrame;
    }

    int getControlledEntityCount() const
    {
        return ((WorldStateHeader *)getPacket())->controlledEntityCount;
    }

    int getLatestClientFrame() const
    {
        return ((WorldStateHeader *)getPacket())->latestClientFrame;
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

    std::unordered_map<int, WorldStateEntityHeader *>& getEntityHeaders()
    {
        return entityHeaders;
    }

       // New method to deserialize DeltaState
    std::shared_ptr<DeltaState> deserializeDeltaState(int id)
    {
        void *buffer = entityData.at(id);
        int size = entityHeaders.at(id)->size;
        auto deltaState = std::make_shared<DeltaState>();
        size_t offset = 0;

        // Ensure the buffer has enough data for changedFields
        if (size < sizeof(uint32_t)) {
            // Handle error: buffer too small
            std::cerr << "Error: Buffer too small to contain changedFields." << std::endl;
            return nullptr;
        }

        // Read changedFields
        memcpy(&deltaState->changedFields, buffer, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Calculate remaining data size
        size_t dataSize = size - offset;

        // Read data
        if (dataSize > 0) {
            deltaState->data.resize(dataSize);
            memcpy(deltaState->data.data(), reinterpret_cast<uint8_t*>(buffer) + offset, dataSize);
        }

        return deltaState;
    }

private:
    int controlledEntityCount;
    int entityCount;

    std::unordered_map<int, bool> allEntities;
    std::unordered_map<int, bool> controlledEntities;
    std::unordered_map<int, WorldStateEntityHeader *> entityHeaders;
    std::unordered_map<int, void*> entityData;
};