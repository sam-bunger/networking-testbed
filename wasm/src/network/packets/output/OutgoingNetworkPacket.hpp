#pragma once

#include <memory>
#include "../Packet.hpp"

class OutgoingNetworkPacket
{
public:
  OutgoingNetworkPacket();

  const void *getPacket();
  int getPacketSize();
  bool isReliable();

  std::shared_ptr<const void> getSharedPacket();

protected:
  void applyCompression();
  void initializeBuffer(int size, int command);
  const void *getData();
  int getSize();
  void setReliability(bool reliable);

private:
  bool reliable;
  std::shared_ptr<const void> packetData;
  int packetSize;
  int originalSize;
};