#pragma once

#include "./OutgoingNetworkPacket.hpp"
#include "../PlayerInputPacket.hpp"
#include <vector>

template<class Input>
class OutgoingPlayerInputPacket : public OutgoingNetworkPacket
{
public:
    OutgoingPlayerInputPacket(std::deque<PlayerInputPacket<Input>> inputs)
    {
        int totalSize = sizeof(PlayerInputPacketHeader) + (inputs.size() * sizeof(PlayerInputPacket<Input>));

        initializeBuffer(totalSize, ReservedCommands::INPUT_UPDATE);

        PlayerInputPacketHeader *header = (PlayerInputPacketHeader *)getData();
        header->size = inputs.size();

        char *current = (char*)getData() + sizeof(PlayerInputPacketHeader);

        for (const auto& input : inputs) {
            PlayerInputPacket<Input> *inputPacket = (PlayerInputPacket<Input> *)current;
            *inputPacket = input;
            current += sizeof(PlayerInputPacket<Input>);
        }

        applyCompression();
    }
};