#include "./ClientSimulationController.hpp"
#include <iostream>

ClientSimulationController::ClientSimulationController() 
    : latestInput() 
    , clientWorld()
    , network()
    , currentEntityIt()
    , controller(&clientWorld, network.clientInterface, ClientConfig())
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
    if (currentEntityIt == controller.getEntities().end()) {
        return -1;
    }
    
    int id = currentEntityIt->second.getEntity()->getId();
    ++currentEntityIt;
    return id;
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