#pragma once

#include "./VisualInterface.hpp"
#include "./game/index.hpp"
#include "./SimulatedNetwork.hpp"
#include "./network/client/ClientController.hpp"

class ClientSimulationController : public VisualInterface
{
public:
    ClientSimulationController();
    ~ClientSimulationController();

    std::weak_ptr<INetwork> getClientNetwork();
    std::weak_ptr<INetwork> getServerNetwork();
    SimulatedNetwork &getNetwork();

    void tick();
    GameInput &getLatestInput();
    int getLastPredictedFrameCount();

    int getDownloadBps();
    int getUploadBps();

    virtual int resetEntityIterator() override;
    virtual int getNextEntityId() override;
    virtual int getEntityType(int id) override;
    virtual PlayerEntity &getPlayerEntity(int id) override;
    virtual BulletEntity &getBulletEntity(int id) override;
    ClientController<GameEntityType, GameEntity, GameInput> &getController() { return controller; }

private:
    GameInput latestInput;
    GameWorld clientWorld;
    SimulatedNetwork network;
    std::unordered_map<int, ClientEntity<GameEntityType, GameEntity, GameInput>>::const_iterator currentEntityIt;
    ClientController<GameEntityType, GameEntity, GameInput> controller;
};