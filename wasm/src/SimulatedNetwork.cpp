#include "./SimulatedNetwork.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ctime>

SimulatedPacket::SimulatedPacket(OutgoingNetworkPacket packet) : packet(packet)
{
    timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

SimulatedNetworkInterface::SimulatedNetworkInterface()
    : packetDropRate(0.0f)
    , packetDelay(0.0f)
    , packetJitter(0.0f)
    , throughputRate(0.0f)
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
    pushTo->push_back(SimulatedPacket(packet));
}

void SimulatedNetworkInterface::setQueues(std::list<SimulatedPacket>* recieveFrom, std::list<SimulatedPacket>* pushTo)
{   
    this->recieveFrom = recieveFrom;
    this->pushTo = pushTo;
}

bool SimulatedNetworkInterface::checkThroughputLimit(size_t packetSize)
{
    if (throughputRate <= 0) return true; // No limit

    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Remove old windows
    while (!throughputWindows.empty() && 
           throughputWindows.front().timestamp < currentTime - WINDOW_SIZE_MS) {
        throughputWindows.erase(throughputWindows.begin());
    }

    // Calculate current throughput
    size_t totalBytes = 0;
    for (const auto& window : throughputWindows) {
        totalBytes += window.bytes;
    }

    // Check if new packet would exceed throughput
    float currentKBps = (totalBytes + packetSize) / 1024.0f;
    if (currentKBps > throughputRate) {
        return false;
    }

    // Add new packet to window
    throughputWindows.push_back({currentTime, packetSize});
    return true;
}

std::vector<NetworkPacket> SimulatedNetworkInterface::getReadyPackets(std::list<SimulatedPacket>& queue)
{
    std::vector<NetworkPacket> readyPackets;
    if (queue.empty()) {
        return readyPackets;
    }

    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    for (auto it = queue.begin(); it != queue.end();)
    {
        uint64_t randomJitter = (uint64_t)((((float)rand() / (float)RAND_MAX)) * (float)packetJitter);
        uint64_t readyTime = it->timestamp + packetDelay + randomJitter;

        if (readyTime < currentTime)
        {
            // Check throughput limit before processing packet
            if (!checkThroughputLimit(it->packet.getPacketSize())) {
                ++it;
                continue; // Skip this packet for now
            }

            if (it->packet.isReliable())
            {
                readyPackets.push_back(NetworkPacket(it->packet.getSharedPacket(), it->packet.getPacketSize()));
            }
            else
            {
                if (((float)rand() / (float)RAND_MAX) > packetDropRate)
                {
                    int packetSize = it->packet.getPacketSize();
                    readyPackets.push_back(NetworkPacket(it->packet.getSharedPacket(), packetSize));
                }
            }
            
            it = queue.erase(it);
        } else {
            ++it;
        }
    }

    return readyPackets;
}

SimulatedNetwork::SimulatedNetwork() 
    : serverInterface(std::make_shared<SimulatedNetworkInterface>())
    , clientInterface(std::make_shared<SimulatedNetworkInterface>())
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
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

void SimulatedNetwork::setThroughputRate(float kbps)
{
    serverInterface->throughputRate = kbps;
    clientInterface->throughputRate = kbps;
}