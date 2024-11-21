#pragma once

template<class Entity>
class EntityWorldInterface
{
public:
    virtual void addEntityToWorld(Entity* entity) = 0;
    virtual void removeEntityFromWorld(Entity* entity) = 0;
};
