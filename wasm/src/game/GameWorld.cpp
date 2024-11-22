#include "./GameWorld.hpp"
#include "./entities/PlayerEntity.hpp"
#include "./entities/BulletEntity.hpp"
#include <cmath>
#include <iostream>
#include <fenv.h>
#include <vector>

GameWorld::GameWorld() : entities()
{
}

GameWorld::~GameWorld()
{

}

void GameWorld::physicsStepHook()
{   
    // Use fixed-point math or ensure consistent floating-point behavior
    #pragma STDC FENV_ACCESS ON
    // Set consistent rounding mode
    fesetround(FE_TONEAREST);
    
    // Sort entity pairs to ensure consistent collision order
    std::vector<std::pair<int, int>> collisionPairs;
    for (auto& [idA, entityA] : entities) {
        for (auto& [idB, entityB] : entities) {
            if (idA < idB) { // Only add each pair once, in consistent order
                collisionPairs.push_back({idA, idB});
            }
        }
    }
    
    // Process collisions in sorted order
    for (const auto& [idA, idB] : collisionPairs) {
        auto& entityA = entities[idA];
        auto& entityB = entities[idB];
        
        if (idA == idB) continue;

        if (entityA->getType() == PLAYER && entityB->getType() == PLAYER)
        {
            PlayerEntity* playerA = static_cast<PlayerEntity*>(entityA);
            PlayerEntity* playerB = static_cast<PlayerEntity*>(entityB);
            
            // Calculate distance between centers
            float dx = playerB->x - playerA->x;
            float dy = playerB->y - playerA->y;
            float distanceSquared = dx*dx + dy*dy;
            
            // Sum of radii
            float minDistance = playerA->radius + playerB->radius;
            
            // Check for collision
            if (distanceSquared < minDistance * minDistance)
            {
                // Calculate actual distance (prevent division by zero)
                float distance = sqrt(distanceSquared);
                if (distance < 0.0001f) // Small epsilon value
                {
                    // Objects are practically at the same position
                    // Push them apart slightly in an arbitrary direction
                    distance = 0.0001f;
                    dx = distance;
                    dy = 0;
                }
                
                // Normalize direction vector
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Calculate relative velocity
                float dvx = playerB->vx - playerA->vx;
                float dvy = playerB->vy - playerA->vy;
                
                // Calculate relative velocity along the normal
                float velAlongNormal = dvx * nx + dvy * ny;
                
                // Remove the early return and only skip collision response
                if (velAlongNormal <= 0)
                {
                    // Collision response (assuming equal mass)
                    float restitution = 0.5f;
                    float j = -(1 + restitution) * velAlongNormal / 2;
                    
                    // Apply impulse
                    playerA->vx -= j * nx;
                    playerA->vy -= j * ny;
                    playerB->vx += j * nx;
                    playerB->vy += j * ny;
                }

                // Add position correction to prevent sinking
                float percent = 0.2f; // Penetration percentage to correct
                float slop = 0.01f;   // Penetration allowance
                float penetration = minDistance - distance;
                if (penetration > slop)
                {
                    float correction = (penetration * percent) / 2.0f;
                    playerA->x -= nx * correction;
                    playerA->y -= ny * correction;
                    playerB->x += nx * correction;
                    playerB->y += ny * correction;
                }
            }
        }
    }
    
    // Update positions in consistent order
    for (auto& [id, entity] : entities) {
        entity->x += entity->vx;
        entity->y += entity->vy;
    }
}

INetworkEntity<GameEntityType, GameEntity, GameInput>* GameWorld::createEntityHook(int id, GameEntityType type, EntityController<GameEntityType, GameEntity>* controller)
{
    switch (type)
    {
        case PLAYER:
            return new PlayerEntity(id, type, controller, this);
        case BULLET:
            return new BulletEntity(id, type, controller, this);
        default:
            return nullptr;
    }
}

void GameWorld::addEntityToWorld(GameEntity* entity) 
{
    entities[entity->getId()] = entity;
}

void GameWorld::removeEntityFromWorld(GameEntity* entity) 
{
    entities.erase(entity->getId());
}
