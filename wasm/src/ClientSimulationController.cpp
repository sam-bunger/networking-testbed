#include "./ClientSimulationController.hpp"
#include <iostream>

ClientSimulationController::ClientSimulationController() 
    : latestInput() 
    , clientWorld()
    , network()
    , currentEntityIt()
    , controller(&clientWorld, network.clientInterface, ClientConfig(60))
{ 
}

ClientSimulationController::~ClientSimulationController() 
{

}

void ClientSimulationController::tick() {
    controller.tick(latestInput);
}

GameInput &ClientSimulationController::getLatestInput() {
    return latestInput;
}

SimulatedNetwork &ClientSimulationController::getNetwork()
{
    return network;
}

int ClientSimulationController::getLastPredictedFrameCount()
{
    return controller.getLastPredictedFrameCount();
}

std::weak_ptr<INetwork> ClientSimulationController::getClientNetwork() {
    return std::weak_ptr<INetwork>(network.clientInterface);
}

std::weak_ptr<INetwork> ClientSimulationController::getServerNetwork() {
    return std::weak_ptr<INetwork>(network.serverInterface);
}

int ClientSimulationController::resetEntityIterator()
{
    currentEntityIt = controller.getEntities().begin();
    return controller.getEntities().size();
}

int ClientSimulationController::getNextEntityId()
{
    if (currentEntityIt == controller.getEntities().end()) 
    {
        return -1;
    }
    
    do 
    {
        auto clientEntity = currentEntityIt->second;
        if (!clientEntity.isMarkedForDeletion()) {
            int id = clientEntity.getEntity()->getId();
            ++currentEntityIt;
            return id;
        }
        ++currentEntityIt;
    } 
    while (currentEntityIt != controller.getEntities().end());
    
    return -1;
}

int ClientSimulationController::getEntityType(int id)
{
    return controller.getEntityById(id)->getType();
}

PlayerEntity &ClientSimulationController::getPlayerEntity(int id)
{
    return *static_cast<PlayerEntity*>(controller.getEntityById(id));
}

BulletEntity &ClientSimulationController::getBulletEntity(int id)
{
    return *static_cast<BulletEntity*>(controller.getEntityById(id));
}

int ClientSimulationController::getDownloadBps()
{
    return network.getIncomingBps();
}

int ClientSimulationController::getUploadBps()
{
    return network.getOutgoingBps();
}