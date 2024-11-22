#pragma once

#pragma pack(push, 1)
struct EntityLifecyclePacket
{
    int serverFrameNumber;
    int id;
    bool created;
    unsigned char type;
};
#pragma pack(pop)