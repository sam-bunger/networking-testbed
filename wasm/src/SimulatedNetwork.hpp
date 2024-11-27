#pragma once

#include <list>
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

    void setQueues(std::list<SimulatedPacket>* recieveFrom, std::list<SimulatedPacket>* pushTo);
    
    int getBps();

    float packetDropRate;
    uint64_t packetJitter;
    uint64_t packetDelay;
    float throughputRate;

    std::list<SimulatedPacket>* recieveFrom;
    std::list<SimulatedPacket>* pushTo;

private:
    std::vector<NetworkPacket> getReadyPackets(std::list<SimulatedPacket>& queue);

    struct ThroughputWindow {
        uint64_t timestamp;
        size_t bytes;
    };
    std::vector<ThroughputWindow> throughputWindows;
    static constexpr size_t WINDOW_SIZE_MS = 1000;

    bool checkThroughputLimit(size_t packetSize);
};


class SimulatedNetwork 
{
public: 
    SimulatedNetwork();

    void setPacketDropRate(float rate);
    void setPacketDelay(float delay);
    void setPacketJitter(float jitter);
    void setThroughputRate(float kbps);

    int getIncomingBps();
    int getOutgoingBps();

    std::list<SimulatedPacket> packetsToServer;
    std::list<SimulatedPacket> packetsToClient;

    std::shared_ptr<SimulatedNetworkInterface> serverInterface;
    std::shared_ptr<SimulatedNetworkInterface> clientInterface;
};