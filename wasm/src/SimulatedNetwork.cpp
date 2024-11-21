#include "./SimulatedNetwork.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>

SimulatedPacket::SimulatedPacket(OutgoingNetworkPacket packet) : packet(packet)
{
    timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

SimulatedNetworkInterface::SimulatedNetworkInterface()
    : packetDropRate(0.0f)
    , packetDelay(0.0f)
    , packetJitter(0.0f)
    , packetReorderRate(0.0f)
{
    recieveFrom = NULL;
    pushTo = NULL;
}

std::vector<NetworkPacket> SimulatedNetworkInterface::getIncomingPackets() 
{
    if (recieveFrom == NULL) {
        return {};
    }
    return getReadyPackets(*recieveFrom);
}

void SimulatedNetworkInterface::pushOutgoingPacket(OutgoingNetworkPacket packet)
{
    if (pushTo == NULL) {
        return;
    }
    pushTo->push(SimulatedPacket(packet));
}

void SimulatedNetworkInterface::setQueues(std::queue<SimulatedPacket>* recieveFrom, std::queue<SimulatedPacket>* pushTo)
{   
    this->recieveFrom = recieveFrom;
    this->pushTo = pushTo;
}

std::vector<NetworkPacket> SimulatedNetworkInterface::getReadyPackets(std::queue<SimulatedPacket>& queue)
{
    std::vector<NetworkPacket> readyPackets;
    if (queue.empty()) {
        return readyPackets;
    }

    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    while (!queue.empty())
    {
        SimulatedPacket packet = queue.front();  
        uint64_t randomJitter = (uint64_t)(((float)rand() / (float)RAND_MAX)) * packetJitter;
        uint64_t readyTime = packet.timestamp + packetDelay + randomJitter;

        if (readyTime < currentTime)
        {
            if (((float)rand() / (float)RAND_MAX) < packetReorderRate)
            {
                packet.timestamp = currentTime + 50 + (rand() % 100);
                queue.push(packet);
                queue.pop();
                continue;
            }

            if (((float)rand() / (float)RAND_MAX) > packetDropRate)
            {
                int packetSize = packet.packet.getPacketSize();
                readyPackets.push_back(NetworkPacket(packet.packet.getSharedPacket(), packetSize));
            }
            queue.pop();
        } 
        else 
        {
            queue.push(queue.front());
            queue.pop();
            break;
        }
    }

    return readyPackets;
}

SimulatedNetwork::SimulatedNetwork() 
    : serverInterface(std::make_shared<SimulatedNetworkInterface>())
    , clientInterface(std::make_shared<SimulatedNetworkInterface>())
{
    serverInterface->setQueues(&packetsToServer, &packetsToClient);
    clientInterface->setQueues(&packetsToClient, &packetsToServer);
}

void SimulatedNetwork::setPacketDropRate(float rate)
{
    serverInterface->packetDropRate = rate;
    clientInterface->packetDropRate = rate;    
}

void SimulatedNetwork::setPacketDelay(float delay)
{
    serverInterface->packetDelay = delay;
    clientInterface->packetDelay = delay;
}

void SimulatedNetwork::setPacketJitter(float jitter)
{
    serverInterface->packetJitter = jitter;
    clientInterface->packetJitter = jitter;
}

void SimulatedNetwork::setPacketReorderRate(float rate)
{
    serverInterface->packetReorderRate = rate;
    clientInterface->packetReorderRate = rate;
}


