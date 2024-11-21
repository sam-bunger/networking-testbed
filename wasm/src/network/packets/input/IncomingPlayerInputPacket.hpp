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
        PlayerInputPacketHeader *header = (PlayerInputPacketHeader *)getPacket();
        numInputs = header->size;

        int expectedSize = sizeof(PlayerInputPacketHeader) + (numInputs * sizeof(PlayerInputPacket<Input>));
        if (getPacketSize() != expectedSize) 
        {
            notValid();
            return;
        }
    }

    std::vector<PlayerInputPacket<Input>> getInputs() const {
        if (!isValid()) return {};
        
        const PlayerInputPacket<Input>* rawInputs = reinterpret_cast<const PlayerInputPacket<Input>*>(
            getPacket() + sizeof(PlayerInputPacketHeader)
        );

        return std::vector<PlayerInputPacket<Input>>(rawInputs, rawInputs + numInputs);
    }

    int getInputCount() const {
        return numInputs;
    }

private:
    int numInputs;
};