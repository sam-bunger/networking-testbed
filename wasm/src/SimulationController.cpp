#include "./SimulationController.hpp"
#include <iostream>

SimulationController::SimulationController() : serverWorld(), 
                                               config(4)
{
    server = new ServerController<GameEntityType, GameEntity, GameInput>(&serverWorld, config);
}

SimulationController::~SimulationController()
{
    delete server;
}

void SimulationController::tick()
{
    for (ClientSimulationController &client : clients)
    {
        client.tick();
    }
    server->tick();
}

ClientSimulationController &SimulationController::createNewClient()
{
    clients.emplace_back();

    ClientSimulationController &client = clients.back();

    std::weak_ptr<INetwork> clientNetwork = client.getServerNetwork();

    auto connection = server->newClient(clientNetwork).lock();

    if (!connection) {
        throw std::runtime_error("Failed to create client connection");
    }

    auto entity = connection->createControllableEntity(GameEntityType::PLAYER);

    client.getController().setControlledEntity(entity->getId());

    return client;
}

void SimulationController::removeClient(ClientSimulationController &client)
{
    clients.remove_if([&client](const ClientSimulationController& x) {
        return &x == &client;
    });
}

int SimulationController::resetEntityIterator()
{
    currentEntityIt = server->getEntities().begin();
    return server->getEntities().size();
}

int SimulationController::getNextEntityId()
{
    if (currentEntityIt == server->getEntities().end()) {
        return -1;
    }
    
    do {
        auto serverEntity = currentEntityIt->second;
        if (serverEntity.isEnabled()) {
            int id = serverEntity.getEntity()->getId();
            ++currentEntityIt;
            return id;
        }
        ++currentEntityIt;
    } while (currentEntityIt != server->getEntities().end());
    
    return -1;

}

int SimulationController::getEntityType(int id)
{
    return server->getEntityById(id)->getType();
}

PlayerEntity &SimulationController::getPlayerEntity(int id)
{
    return *static_cast<PlayerEntity*>(server->getEntityById(id));
}

BulletEntity &SimulationController::getBulletEntity(int id)
{
    return *static_cast<BulletEntity*>(server->getEntityById(id));
}