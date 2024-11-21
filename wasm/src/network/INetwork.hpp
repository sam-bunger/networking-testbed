#pragma once

#include <vector>
#include <memory>
#include "./packets/output/OutgoingNetworkPacket.hpp"
#include "./packets/input/IncomingNetworkPacket.hpp"

class NetworkPacket {
public:
    NetworkPacket() : packetData(nullptr), packetSize(0) {}
    NetworkPacket(std::shared_ptr<const void> data, int size) : packetData(data), packetSize(size) {}

    std::shared_ptr<const void> getPacketData() { return packetData; }
    int getPacketSize() { return packetSize; }
private:
    std::shared_ptr<const void> packetData;
    int packetSize;
};

class INetwork {
public:
    virtual ~INetwork() {}

    virtual std::vector<NetworkPacket> getIncomingPackets() = 0; 
    virtual void pushOutgoingPacket(OutgoingNetworkPacket packet) = 0;
};