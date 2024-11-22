#pragma once

#pragma pack(1)
enum ReservedCommands
{
  WORLD_UPDATE,
  INPUT_UPDATE,
  ENTITY_LIFECYCLE,
};

struct PacketHeader
{
  char command;
  bool compressed;
  unsigned short actualSize;
  unsigned short originalSize;
};
#pragma pack()