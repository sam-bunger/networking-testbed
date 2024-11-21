#pragma once

#include <iterator>
#include <unordered_map>
#include <memory>
#include <list>
#include "./network/index.hpp"
#include "./game/index.hpp"
#include "./VisualInterface.hpp"
#include "./ClientSimulationController.hpp"

class SimulationController : public VisualInterface
{
public:
    SimulationController();
    ~SimulationController();

    virtual int resetEntityIterator() override;
    virtual int getNextEntityId() override;
    virtual int getEntityType(int id) override;
    virtual PlayerEntity &getPlayerEntity(int id) override;
    virtual BulletEntity &getBulletEntity(int id) override;

    ClientSimulationController &createNewClient();
    void removeClient(ClientSimulationController &client);

    void tick();

private:
    GameWorld serverWorld;

    ServerConfig config;
   
    ServerController<GameEntityType, GameEntity, GameInput> *server;
    
    std::unordered_map<int, ServerEntity<GameEntityType, GameEntity, GameInput>>::const_iterator currentEntityIt;

    std::list<ClientSimulationController> clients;
};

