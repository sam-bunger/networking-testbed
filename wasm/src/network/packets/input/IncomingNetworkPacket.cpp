#include "./IncomingNetworkPacket.hpp"
#include "../Packet.hpp"
#include "../../util/Compression.hpp"

#include <cstring>
#include <iostream>

IncomingNetworkPacket::IncomingNetworkPacket(const void *p, int size) 
  : size(0), 
    isPacketValid(true)
{
    PacketHeader *packet = (PacketHeader *)p;
    this->size = packet->originalSize;
    this->packetData = std::shared_ptr<char[]>(new char[packet->originalSize]);

    if (!packet->compressed)
    {
      // Uncompressed packet
      memcpy(
        (char *)this->packetData.get(), 
        (char *)p + sizeof(PacketHeader), 
        packet->originalSize
      );
    }
    else
    {
      // Compressed packet
      decompress(
        (char*)p + sizeof(PacketHeader), 
        size - sizeof(PacketHeader), 
        (char*)this->packetData.get(), 
        packet->originalSize
      );
    }
}

const char* IncomingNetworkPacket::getPacket() const
{
    return this->packetData.get();
}

int IncomingNetworkPacket::getPacketSize() const
{
    return this->size;
}

void IncomingNetworkPacket::notValid()
{
    this->isPacketValid = false;
}

bool IncomingNetworkPacket::isValid() const
{
    return this->isPacketValid;
}