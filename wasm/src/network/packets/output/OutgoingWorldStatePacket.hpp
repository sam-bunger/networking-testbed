#pragma once

#include "./OutgoingNetworkPacket.hpp"
#include "../../INetworkEntity.hpp"
#include "../WorldStatePacket.hpp"
#include <map>

template<typename EntityType, class Entity, class Input>
class OutgoingWorldStatePacket : public OutgoingNetworkPacket
{
public:
    OutgoingWorldStatePacket(int frameNumber, const std::unordered_map<int, PlayerInputPacket<Input>> &inputs, const std::unordered_map<int, ServerEntity<EntityType, Entity, Input>> &entities)
    {
        int controlledEntityCount = inputs.size();
        int entityCount = entities.size() - controlledEntityCount;

        int controlledEntityHeaderSize = (sizeof(WorldStateControlledEntityHeader<Input>) * controlledEntityCount);
        int entityHeaderSize = (sizeof(WorldStateEntityHeader) * entityCount);
        int totalSize = sizeof(WorldStateHeader) + controlledEntityHeaderSize + entityHeaderSize;

        for (const auto& [id, entity] : entities) {
            totalSize += entity.getEntity()->serializeSize();
        }
        
        initializeBuffer(totalSize, ReservedCommands::WORLD_UPDATE);
        
        WorldStateHeader *header = (WorldStateHeader *)getData();
        header->serverFrameNumber = frameNumber;
        header->controlledEntityCount = controlledEntityCount;
        header->entityCount = entityCount;

        char *current = (char*)getData() + sizeof(WorldStateHeader);

        // First loop: Process controlled entities
        for (const auto& [id, entity] : entities) {
            if (inputs.find(entity.getEntity()->getId()) != inputs.end()) {
                WorldStateControlledEntityHeader<Input> *controlledEntityHeader = (WorldStateControlledEntityHeader<Input> *)current;
                controlledEntityHeader->id = entity.getEntity()->getId();
                controlledEntityHeader->size = entity.getEntity()->serializeSize();
                controlledEntityHeader->type = entity.getEntity()->getType();
                controlledEntityHeader->input = inputs.at(entity.getEntity()->getId()).input;
                current += sizeof(WorldStateControlledEntityHeader<Input>);

                entity.getEntity()->serialize(current);
                current += entity.getEntity()->serializeSize();
            }
        }

        // Second loop: Process regular entities
        for (const auto& [id, entity] : entities) {
            if (inputs.find(entity.getEntity()->getId()) == inputs.end()) {
                WorldStateEntityHeader *entityHeader = (WorldStateEntityHeader *)current;
                entityHeader->id = entity.getEntity()->getId();
                entityHeader->size = entity.getEntity()->serializeSize();
                entityHeader->type = entity.getEntity()->getType();
                current += sizeof(WorldStateEntityHeader);

                entity.getEntity()->serialize(current);
                current += entity.getEntity()->serializeSize();
            }
        }

        applyCompression();
    }
};