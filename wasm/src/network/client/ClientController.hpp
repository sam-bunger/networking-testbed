#pragma once

#include <unordered_map>
#include <deque>
#include <iostream>
#include <vector>
#include "../INetwork.hpp"
#include "../INetworkWorldController.hpp"
#include "./ClientEntity.hpp"
#include "../packets/PlayerInputPacket.hpp"
#include "../packets/output/OutgoingPlayerInputPacket.hpp"
#include "../packets/input/IncomingWorldStatePacket.hpp"

struct ClientConfig 
{};

template<typename EntityType, class Entity, class Input>
class ClientController : public INetworkWorldController<EntityType, Entity, Input>
{
public:
    ClientController(
        IPhysicsWorld<EntityType, Entity, Input> *world, 
        std::weak_ptr<INetwork> network,
        ClientConfig config
    ) : INetworkWorldController<EntityType, Entity, Input>(world), 
        config(config),
        network(network),
        emptyInput(),
        entities(),
        controllerId(-1),
        latestInputs(),
        latestWorldState(NULL)
	{ }


    virtual Entity* createEntity(EntityType type) override
	{
		int id = this->getNewEntityId(type);
		return (Entity*) createEntity(id, type);
	}

	virtual Entity* createEntity(int id, EntityType type) override
	{
		auto newEntity = ClientEntity<EntityType, Entity, Input>(this->world->createEntityHook(id, type, this));
		auto [it, inserted] = entities.insert({id, newEntity});
		return (Entity*) it->second.getEntity();
	}

    virtual void destroyEntity(int id) override
	{
		entities.erase(id);
	}

    virtual Entity* getEntityById(int id) override
	{
		auto it = entities.find(id);
		if (it == entities.end()) 
			return NULL;
		return (Entity*) it->second.getEntity();
	}

	virtual bool entityExists(int id) override
	{
		return entities.find(id) != entities.end();
	}

    void setControlledEntity(int id)
    {
        controllerId = id;
    }

    void tick(const Input &input)
    {
        processLatestInputs(input);
        sendInputUpdate();
        processIncomingMessages();
        reconcile();

        INetworkWorldController<EntityType, Entity, Input>::tick();

        updateEntites(input);
        this->world->physicsStepHook();
    }

    const std::unordered_map<int, ClientEntity<EntityType, Entity, Input>>& getEntities()
	{
		return entities;
	}

private:

    void processIncomingMessages()
    {
        latestWorldState = nullptr;

        auto networkPtr = network.lock();
			
        if (!networkPtr) return;

        std::vector<NetworkPacket> messages = networkPtr->getIncomingPackets();

        for (NetworkPacket& message : messages) {
            std::shared_ptr<const void> packet = message.getPacketData();
            PacketHeader *header = (PacketHeader *)packet.get();
            switch (header->command) {
                case ReservedCommands::WORLD_UPDATE: 
                {
                    auto packet = std::make_shared<IncomingWorldStatePacket<EntityType, Entity, Input>>(message.getPacketData().get(), message.getPacketSize());
                    if (packet->isValid()) {
                       if (latestWorldState == nullptr || packet->getServerFrame() > latestWorldState->getServerFrame()) {
                           latestWorldState = packet;
                       }
                    }
                    continue;
                }
                default:
                    continue;
            }
        }
    }

    void reconcile()
    {
        if (latestWorldState == NULL) return;

        int verifiedFrame = latestWorldState->getServerFrame();

        int predictedFrame = this->frameNumber;

        // Roll everything back to the verified frame
        for (auto& [id, entity] : entities) {
            if (latestWorldState->isEntityInPacket(id))
            {
                WorldStateEntityHeader *header = latestWorldState->getEntityHeader(id);
                if (header->type != entity.getEntity()->getType()) {
                    // error - entity type mismatch - figure out way to handle this case
                    continue;
                }

                if (latestWorldState->isControlledEntity(id)) {
                    WorldStateControlledEntityHeader<Input> *controlledHeader = latestWorldState->getControlledEntityHeader(id);
                    entity.timeline.serializeInputOntoTimeline(verifiedFrame, controlledHeader->input);
                }

                entity.getEntity()->deserialize(latestWorldState->getEntityData(id));
            }
            else
            {
                entity.timeline.deserializeFromTimeline(verifiedFrame, *entity.getEntity());
            }

            entity.timeline.clearBeforeFrame(verifiedFrame);
        }

        this->frameNumber = verifiedFrame;
        for (int f = verifiedFrame + 1; f <= predictedFrame; f++) {
            INetworkWorldController<EntityType, Entity, Input>::tick();

            // Reapply inputs
            for (auto& [id, entity] : entities) {
                const Input &input = entity.timeline.getInput(f);
                entity.getEntity()->update(input);
            }

            // Reapply physics
            this->world->physicsStepHook();

            
            // Store new state
            for (auto& [id, entity] : entities) {
                entity.timeline.serializeDataOntoTimeline(f, *entity.getEntity());
            }
        }
    }

    void sendInputUpdate()
    {
        if (controllerId == -1) return;

        OutgoingPlayerInputPacket<Input> packet(latestInputs);

        auto networkPtr = network.lock();
        if (networkPtr) 
        {
            networkPtr->pushOutgoingPacket(packet);
        }
    }

    void processLatestInputs(const Input &input)
    {
        latestInputs.push_back(PlayerInputPacket<Input>(this->getFrameNumber(), input));
        if (latestInputs.size() > 20) {
            latestInputs.pop_front();
        }
    }

    void updateEntites(const Input &input)
    {
        if (controllerId != -1 && entityExists(controllerId)) {
            auto& entity = entities.at(controllerId);
            entity.getEntity()->update(input);
        }

        for (auto& [id, entity] : entities) {
            if (id != controllerId) {
                entity.getEntity()->update(emptyInput);
            }
        }
    }

    ClientConfig config;
    std::weak_ptr<INetwork> network;
    const Input emptyInput;
    std::unordered_map<int, ClientEntity<EntityType, Entity, Input>> entities;
    int controllerId;
    std::deque<PlayerInputPacket<Input>> latestInputs; 
    std::shared_ptr<IncomingWorldStatePacket<EntityType, Entity, Input>> latestWorldState;
};