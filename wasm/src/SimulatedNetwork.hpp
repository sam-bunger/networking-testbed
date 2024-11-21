#pragma once

#include <queue>
#include <functional>
#include "./network/INetwork.hpp"

class SimulatedPacket
{
public:
    SimulatedPacket(OutgoingNetworkPacket packet);
    uint64_t timestamp;
    OutgoingNetworkPacket packet;
};

class SimulatedNetworkInterface : public INetwork
{
public:
    SimulatedNetworkInterface();

    virtual std::vector<NetworkPacket> getIncomingPackets() override; 
    virtual void pushOutgoingPacket(OutgoingNetworkPacket packet) override;

    void setQueues(std::queue<SimulatedPacket>* recieveFrom, std::queue<SimulatedPacket>* pushTo);

    float packetDropRate;
    float packetReorderRate;
    uint64_t packetJitter;
    uint64_t packetDelay;

    std::queue<SimulatedPacket>* recieveFrom;
    std::queue<SimulatedPacket>* pushTo;

private:
    std::vector<NetworkPacket> getReadyPackets(std::queue<SimulatedPacket>& queue);

};

class SimulatedNetwork 
{
public: 
    SimulatedNetwork();

    void setPacketDropRate(float rate);
    void setPacketDelay(float delay);
    void setPacketJitter(float jitter);
    void setPacketReorderRate(float rate);

    std::queue<SimulatedPacket> packetsToServer;
    std::queue<SimulatedPacket> packetsToClient;

    std::shared_ptr<SimulatedNetworkInterface> serverInterface;
    std::shared_ptr<SimulatedNetworkInterface> clientInterface;
};