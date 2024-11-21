#pragma once

#include "./game/index.hpp"
#include "./game/entities/PlayerEntity.hpp"
#include "./game/entities/BulletEntity.hpp"

class VisualInterface 
{
public:
    virtual ~VisualInterface() {}

    virtual int resetEntityIterator() = 0;
    virtual int getNextEntityId() = 0;
    virtual int getEntityType(int id) = 0;
    virtual PlayerEntity &getPlayerEntity(int id) = 0;
    virtual BulletEntity &getBulletEntity(int id) = 0;

};
