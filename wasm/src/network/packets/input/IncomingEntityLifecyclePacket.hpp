#pragma once

#include "./IncomingNetworkPacket.hpp"
#include "../EntityLifecyclePacket.hpp"
#include <vector>
#include <iostream>

enum EntityLifecycleType
{
    CREATED,
    DESTROYED
};

template<typename EntityType>
class IncomingEntityLifecyclePacket : public IncomingNetworkPacket
{
public:
    IncomingEntityLifecyclePacket(const void *packet, int size) : IncomingNetworkPacket(packet, size)
    { 
        
    }

    EntityLifecycleType getLifecycleType() const 
    {
        return ((EntityLifecyclePacket *)getPacket())->created ? EntityLifecycleType::CREATED : EntityLifecycleType::DESTROYED;
    }

    int getEntityId() const 
    {
        return ((EntityLifecyclePacket *)getPacket())->id;
    }   

    EntityType getEntityType() const 
    {
        return (EntityType) ((EntityLifecyclePacket *)getPacket())->type;
    }

    int getFrameNumber() const
    {
        return ((EntityLifecyclePacket *)getPacket())->serverFrameNumber;
    }
};