#pragma once

#include <iostream>
#include <string>
#include "./IPhysicsWorld.hpp"
#include "./IEntityController.hpp"

const int MinEntityId = 10000;
const int MaxEntityId = 999999;

template<typename EntityType, class Entity, class Input>
class INetworkWorldController : public InternalEntityController<EntityType, Entity>
{
    static_assert(std::is_enum_v<EntityType>, "EntityType must be an enum");

public:
    INetworkWorldController(IPhysicsWorld<EntityType, Entity, Input> *world) : world(world), frameNumber(0), nextId(0) {};
    virtual ~INetworkWorldController() {};

    virtual int getFrameNumber() override
	{
		return frameNumber;
	}

    int getNewEntityId(EntityType type) override
    {
        return getValidEntityId(type);
    }

    virtual void tick() 
    {
        nextId = 0;
        frameNumber++;
    }


    int getValidEntityId(EntityType type)
    {
        while (true)
        {
            std::string str = std::to_string(frameNumber) + "_" + std::to_string(type) + "_" + std::to_string(nextId++);

            int newId = simpleHash(str);
            if (!this->entityExists(newId))
            {
                return newId;
            }
        }
    }

    unsigned int simpleHash(const std::string &str)
    {
        unsigned int hash = 0;
        for (char c : str)
        {
            hash = hash * 101 + c;
        }
        return MinEntityId + (hash % (MaxEntityId - MinEntityId));
    }

    
    IPhysicsWorld<EntityType, Entity, Input> *world;
    int frameNumber;
    int nextId;
};