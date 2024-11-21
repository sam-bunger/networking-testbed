#include "./OutgoingNetworkPacket.hpp"
#include "../../util/Compression.hpp"
#include <cstring>
#include <iostream>

const int CompressionThreshold = 1000;

OutgoingNetworkPacket::OutgoingNetworkPacket()
{
  packetSize = 0;
  originalSize = 0;
  reliable = true;
}

void OutgoingNetworkPacket::applyCompression()
{
  if (packetSize < CompressionThreshold || !packetData)
    return;

  auto tempBuffer = std::make_unique<char[]>(sizeof(PacketHeader) + packetSize);
  int compressedSize = compress(sizeof(PacketHeader) + static_cast<const char*>(packetData.get()), 
                               packetSize, 
                               sizeof(PacketHeader) + tempBuffer.get());

  const PacketHeader *originalHeader = reinterpret_cast<const PacketHeader*>(packetData.get());
  PacketHeader *compressedHeader = reinterpret_cast<PacketHeader*>(tempBuffer.get());

  compressedHeader->command = originalHeader->command;
  compressedHeader->compressed = true;
  compressedHeader->actualSize = compressedSize;
  compressedHeader->originalSize = packetSize;

  originalSize = packetSize;
  packetSize = compressedSize;
  packetData = std::shared_ptr<const void>(tempBuffer.release(), std::default_delete<char[]>());
}

void OutgoingNetworkPacket::initializeBuffer(int size, int command)
{
  auto tempBuffer = std::make_unique<char[]>(sizeof(PacketHeader) + size);
  std::memset(tempBuffer.get(), 0, sizeof(PacketHeader) + size);

  PacketHeader *header = reinterpret_cast<PacketHeader*>(tempBuffer.get());
  header->command = command;
  header->compressed = false;
  header->actualSize = size;
  header->originalSize = size;

  packetData = std::shared_ptr<const char[]>(tempBuffer.release());
  packetSize = size;
  originalSize = size;
}

std::shared_ptr<const void> OutgoingNetworkPacket::getSharedPacket()
{
  return packetData;
}

const void *OutgoingNetworkPacket::getPacket()
{
  return packetData.get();
}

int OutgoingNetworkPacket::getPacketSize()
{
  return sizeof(PacketHeader) + packetSize;
}

bool OutgoingNetworkPacket::isReliable()
{
  return reliable;
}

const void *OutgoingNetworkPacket::getData()
{
  return static_cast<const char*>(packetData.get()) + sizeof(PacketHeader);
}

int OutgoingNetworkPacket::getSize()
{
  return packetSize;
}

void OutgoingNetworkPacket::setReliability(bool reliable)
{
  this->reliable = reliable;
}
