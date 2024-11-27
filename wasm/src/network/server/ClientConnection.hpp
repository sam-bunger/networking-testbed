#pragma once

#include <memory>
#include <iostream> 
#include "../INetwork.hpp"
#include "./inputBuffer/InputBuffer.hpp"

template<typename EntityType, class Entity, class Input>
class ClientConnection 
{
public:
    ClientConnection(
        std::weak_ptr<INetwork> network, 
        InternalEntityController<EntityType, Entity> *controller, 
        int id) : network(network), 
                  controller(controller), 
                  assignedEntityId(id),
                  lastAcknowledgedFrame(-1)
    { }
    
    Entity *createControllableEntity(EntityType type) 
	{
        return controller->createEntity(assignedEntityId, type);
	}

    void removeControllableEntity()
    {
        controller->destroyEntity(assignedEntityId);
    }

    void setLastAcknowledgedFrame(int frame)
    {
        if (frame > controller->getFrameNumber() || frame < lastAcknowledgedFrame) return;
        lastAcknowledgedFrame = frame;
    }

    int getLastAcknowledgedFrame()
    {
        return lastAcknowledgedFrame;
    }

    std::weak_ptr<INetwork> network;
    const int assignedEntityId;
    InputBuffer<Input> inputBuffer;

private:
    InternalEntityController<EntityType, Entity> *controller;
    int lastAcknowledgedFrame;
};