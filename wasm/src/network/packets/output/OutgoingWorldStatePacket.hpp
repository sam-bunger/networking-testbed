#pragma once

#include "./OutgoingNetworkPacket.hpp"
#include "../../INetworkEntity.hpp"
#include "../WorldStatePacket.hpp"
#include <map>

template<typename EntityType, class Entity, class Input>
class OutgoingWorldStatePacket : public OutgoingNetworkPacket
{
public:
    OutgoingWorldStatePacket(
        int initialFrameNumber, 
        int finalFrameNumber, 
        int playerEntityId, 
        const std::unordered_map<int, BufferedInput<Input>> &inputs, 
        std::unordered_map<int, ServerEntity<EntityType, Entity, Input>> &entities
    )
    {
        int controlledEntityCount = 0;
        int entityCount = 0;
        int totalSize = 0;

        std::map<int, WorldStateEntityPacketType> entityPacketTypes;
        std::map<int, std::shared_ptr<DeltaState>> entityDeltas;

        // Loop through and calculate entity deltas along with the total size of the packet
        for (auto& [id, entity] : entities) {
            auto packetType = entity.getHistory().getPacketType(initialFrameNumber, finalFrameNumber);

            entityPacketTypes[id] = packetType;

            if (packetType == WorldStateEntityPacketType::ENTITY_SKIPPED)
            {
                continue;
            }

            if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_DELTA) 
            {
                entityDeltas[id] = entity.getHistory().getDiffState(initialFrameNumber, finalFrameNumber);
                if (entityDeltas[id]->isEmpty()) 
                {
                    entityPacketTypes[id] = WorldStateEntityPacketType::ENTITY_SKIPPED;
                    continue;
                }
                totalSize += entityDeltas[id]->size();
            }
            else
            {
                totalSize += entity.getEntity()->serializeSize();
            }

            if (inputs.find(entity.getEntity()->getId()) != inputs.end()) 
            {
                controlledEntityCount++;
            } 
            else 
            {
                entityCount++;
            }
        }

        // Add the size of the headers
        totalSize += sizeof(WorldStateHeader) + (sizeof(WorldStateControlledEntityHeader<Input>) * controlledEntityCount) + (sizeof(WorldStateEntityHeader) * entityCount);

        initializeBuffer(totalSize, ReservedCommands::WORLD_UPDATE);

        WorldStateHeader *header = (WorldStateHeader *)getData();

        if (playerEntityId != -1) {
            header->latestClientFrame = inputs.at(playerEntityId).latestClientFrame;
        } else {
            header->latestClientFrame = -1;
        }

        header->initialServerFrame = initialFrameNumber;
        header->finalServerFrame = finalFrameNumber;
        header->controlledEntityCount = controlledEntityCount;
        header->entityCount = entityCount;

        char *current = (char*)getData() + sizeof(WorldStateHeader);

        for (const auto& [id, entity] : entities) {
            if (inputs.find(entity.getEntity()->getId()) != inputs.end()) {

                if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_SKIPPED)
                {
                    continue;
                }

                WorldStateControlledEntityHeader<Input> *controlledEntityHeader = (WorldStateControlledEntityHeader<Input> *)current;
                controlledEntityHeader->id = entity.getEntity()->getId();
                controlledEntityHeader->type = entity.getEntity()->getType();
                controlledEntityHeader->input = inputs.at(entity.getEntity()->getId()).input;
                controlledEntityHeader->entityPacketType = entityPacketTypes[id];
                current += sizeof(WorldStateControlledEntityHeader<Input>);
                
                if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_DELTA) 
                {
                    int size = entityDeltas[id]->size();
                    memcpy(current, &(entityDeltas[id]->changedFields), sizeof(uint32_t));
                    current += sizeof(uint32_t);

                    memcpy(current, entityDeltas[id]->data.data(), entityDeltas[id]->data.size());
                    current += entityDeltas[id]->data.size();

                    controlledEntityHeader->size = size;
                }
                else if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_FULL_RELOAD)
                {
                    entity.getEntity()->serialize(current);
                    int size = entity.getEntity()->serializeSize();
                    controlledEntityHeader->size = size;
                    current += size;
                }
                else if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_DELETED)
                {
                    controlledEntityHeader->size = 0;
                }
            }
        }

        for (const auto& [id, entity] : entities) {
            if (inputs.find(entity.getEntity()->getId()) == inputs.end()) {

                if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_SKIPPED)
                {
                    continue;
                }

                WorldStateEntityHeader *entityHeader = (WorldStateEntityHeader *)current;
                entityHeader->id = entity.getEntity()->getId();
                entityHeader->type = entity.getEntity()->getType();
                entityHeader->entityPacketType = entityPacketTypes[id];
                current += sizeof(WorldStateEntityHeader);

                if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_DELTA) 
                {
                    int size = entityDeltas[id]->size();

                    // Copy changedFields
                    memcpy(current, &(entityDeltas[id]->changedFields), sizeof(uint32_t));
                    current += sizeof(uint32_t);

                    // Copy data
                    memcpy(current, entityDeltas[id]->data.data(), entityDeltas[id]->data.size());
                    current += entityDeltas[id]->data.size();

                    entityHeader->size = size;
                }
                else if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_FULL_RELOAD)
                {
                    entity.getEntity()->serialize(current);
                    int size = entity.getEntity()->serializeSize();
                    entityHeader->size = size;
                    current += size;
                }
                else if (entityPacketTypes[id] == WorldStateEntityPacketType::ENTITY_DELETED)
                {
                    entityHeader->size = 0;
                }
            }
        }

        applyCompression();
    }
};
