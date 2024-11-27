#pragma once

#include "./IncomingNetworkPacket.hpp"
#include "../PlayerInputPacket.hpp"
#include <vector>
#include <iostream>

template<class Input>
class IncomingPlayerInputPacket : public IncomingNetworkPacket
{
public:
    IncomingPlayerInputPacket(const void *packet, int size) : IncomingNetworkPacket(packet, size)
    {
        int expectedSize = sizeof(PlayerInputPacketHeader) + (getInputCount() * sizeof(PlayerInputPacket<Input>));
        if (getPacketSize() != expectedSize) 
        {
            notValid();
            return;
        }
    }

    std::vector<PlayerInputPacket<Input>> getInputs() const 
    {
        if (!isValid()) return {};
        
        const PlayerInputPacket<Input>* rawInputs = reinterpret_cast<const PlayerInputPacket<Input>*>(
            getPacket() + sizeof(PlayerInputPacketHeader)
        );

        return std::vector<PlayerInputPacket<Input>>(rawInputs, rawInputs + getInputCount());
    }

    int getInputCount() const 
    {
        return ((PlayerInputPacketHeader *)getPacket())->size;
    }

    int getLastAcknowledgedFrame() const 
    {
        return ((PlayerInputPacketHeader *)getPacket())->acknowledgedFrame;
    }
};