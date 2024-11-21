#pragma once

#include "./PlayerInputPacket.hpp"

#pragma pack(push, 1)
struct WorldStateHeader
{
    int serverFrameNumber;
    int controlledEntityCount;
    int entityCount;
};

struct WorldStateEntityHeader
{
    int id;
    short size;
    unsigned char type;
};

template<class Input>
struct WorldStateControlledEntityHeader : WorldStateEntityHeader
{
    Input input;
};
#pragma pack(pop)