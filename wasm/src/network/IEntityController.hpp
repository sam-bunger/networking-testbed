#pragma once

#include <memory>

template<typename EntityType, class Entity>
class EntityController {
public:
    virtual ~EntityController() {};
    virtual int getFrameNumber() = 0;
    virtual Entity* createEntity(EntityType type)  = 0;
    virtual void destroyEntity(int id) = 0;
    virtual bool entityExists(int id) = 0;
    virtual Entity* getEntityById(int id) = 0;
};

template<typename EntityType, class Entity>
class InternalEntityController : public EntityController<EntityType, Entity> {
public:
    virtual ~InternalEntityController() {};
    virtual int getNewEntityId(EntityType type) = 0;
    virtual Entity* createEntity(int id, EntityType type)  = 0;
};
