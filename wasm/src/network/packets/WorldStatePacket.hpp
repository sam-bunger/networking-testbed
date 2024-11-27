#pragma once

#include "./PlayerInputPacket.hpp"

enum WorldStateEntityPacketType
{
    ENTITY_DELTA,
    ENTITY_FULL_RELOAD,
    ENTITY_DELETED,
    ENTITY_SKIPPED
};

#pragma pack(push, 1)
struct WorldStateHeader
{
    int initialServerFrame;
    int finalServerFrame;
    int latestClientFrame;
    unsigned short controlledEntityCount;
    unsigned short entityCount;
};

struct WorldStateEntityHeader
{
    int id;
    short size;
    unsigned char type;
    unsigned char entityPacketType;
};

template<class Input>
struct WorldStateControlledEntityHeader : WorldStateEntityHeader
{
    Input input;
};
#pragma pack(pop)