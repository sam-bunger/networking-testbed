#pragma once

#include <vector>
#include "./OutgoingNetworkPacket.hpp"
#include "../EntityLifecyclePacket.hpp"
#include "../../INetworkEntity.hpp"

template<typename EntityType, class Entity, class Input>
class OutgoingEntityLifecyclePacket : public OutgoingNetworkPacket
{
public:
    OutgoingEntityLifecyclePacket(int frameNumber, bool created, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        int totalSize = sizeof(EntityLifecyclePacket);

        initializeBuffer(totalSize, ReservedCommands::ENTITY_LIFECYCLE);

        EntityLifecyclePacket *header = (EntityLifecyclePacket *)getData();
        header->serverFrameNumber = frameNumber;
        header->created = created;
        header->id = entity.getId();
        header->type = entity.getType();

        setReliability(true);
    }
};