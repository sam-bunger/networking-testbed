#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "../INetwork.hpp"
#include "../INetworkWorldController.hpp"
#include "./inputBuffer/InputBuffer.hpp"
#include "./ServerEntity.hpp"
#include "../packets/input/IncomingPlayerInputPacket.hpp"
#include "../packets/output/OutgoingWorldStatePacket.hpp"
#include "./ClientConnection.hpp"

const int MinPlayerId = 1000;
const int MaxPlayerId = 99999;

struct ServerConfig 
{
	ServerConfig(int networkUpdateRate) : networkUpdateRate(networkUpdateRate) {}

	int networkUpdateRate;
};

template<typename EntityType, class Entity, class Input>
class ServerController : public INetworkWorldController<EntityType, Entity, Input>
{
public:
	ServerController(IPhysicsWorld<EntityType, Entity, Input> *world, ServerConfig &config) 
		: INetworkWorldController<EntityType, Entity, Input>(world),
		  config(config)
	{}

    virtual Entity* createEntity(EntityType type) override
	{
		int id = this->getNewEntityId(type);
		return createEntity(id, type);
	}

	virtual Entity* createEntity(int id, EntityType type) override
	{
		auto entityIt = entities.find(id);
		if (entityIt == entities.end()) 
		{
			auto newEntity = ServerEntity<EntityType, Entity, Input>(this->world->createEntityHook(id, type, this));
			auto [it, inserted] = entities.insert({id, newEntity});
			entityIt = it;
		}

		entityIt->second.setReadyForDeletion(false);
		entityIt->second.getEntity()->addToWorld();

		return (Entity*) entityIt->second.getEntity();
	}

    virtual void destroyEntity(int id) override
	{
		auto it = entities.find(id);
		if (it == entities.end())
			return;

		it->second.setReadyForDeletion(true);
		it->second.getEntity()->removeFromWorld();
	}

    virtual Entity* getEntityById(int id) override
	{
		auto it = entities.find(id);
		if (!(it != entities.end() && it->second.isEnabled())) 
			return NULL;
		
		return (Entity*) it->second.getEntity();
	}

	virtual bool entityExists(int id) override
	{
		auto it = entities.find(id);
		return it != entities.end() && it->second.isEnabled();
	}

    std::weak_ptr<ClientConnection<EntityType, Entity, Input>> newClient(std::weak_ptr<INetwork> clientNetwork) 
	{
        int id = this->getNewControllerId();
		auto client = std::make_shared<ClientConnection<EntityType, Entity, Input>>(clientNetwork, this, id);
		clients[id] = client;
		return clients[id];
    }

    virtual void tick() override
	{
		INetworkWorldController<EntityType, Entity, Input>::tick();
		processIncomingMessages();
		updateEntities();
		pruneEntities();
		this->world->physicsStepHook();
		processWorldState();
		sendOutgoingMessages();
    }

	const std::unordered_map<int, ServerEntity<EntityType, Entity, Input>>& getEntities()
	{
		return entities;
	}

private:
	int getNewControllerId()
	{
		static int nextControllerId = MinPlayerId;
		while (nextControllerId <= MaxPlayerId) {
			if (std::none_of(clients.begin(), clients.end(), 
				[id = nextControllerId](const auto& client) { return client.second->assignedEntityId == id; })) {
				return nextControllerId++;
			}
			nextControllerId++;
		}
		throw std::runtime_error("No available player IDs");
	}

    void processIncomingMessages() {
        for (auto& [_, client] : clients) {
			auto clientNetwork = client->network.lock();
			
			if (!clientNetwork) 
				continue;

            std::vector<NetworkPacket> messages = clientNetwork->getIncomingPackets();

            for (NetworkPacket& message : messages) {
                std::shared_ptr<const void> packet = message.getPacketData();
				PacketHeader *header = (PacketHeader *)packet.get();
                switch (header->command) {
                    case ReservedCommands::INPUT_UPDATE: {
                        IncomingPlayerInputPacket<Input> packet(message.getPacketData().get(), message.getPacketSize());
                        if (packet.isValid()) {
                            client->inputBuffer.addInputs(packet.getInputs());
							client->setLastAcknowledgedFrame(packet.getLastAcknowledgedFrame());
                        }
                        continue;
                    }
                    default:
                        continue;
                }
            }
        }
    }

	void updateEntities()
	{
		controlledEntityInputs.clear();

		// Update Player Controlled Entities
		for (auto it = clients.begin(); it != clients.end();) {
			auto& client = *it->second;
			
			// Remove client if network connection is lost
			if (client.network.expired()) {
				it = clients.erase(it);
				continue;
			}

			BufferedInput<Input> input = client.inputBuffer.getNextInput(this->frameNumber);

			if (client.assignedEntityId != -1) 
			{
				ServerEntity<EntityType, Entity, Input> &e = entities.at(client.assignedEntityId);
				
				if (!e.isEnabled()) 
					continue;

				e.getEntity()->update(input.input);
				controlledEntityInputs[e.getEntity()->getId()] = input;
			}
			
			++it;
		}

		// Update Non-Player Entities
		for (auto& [id, entity] : entities) {
			if (controlledEntityInputs.find(id) == controlledEntityInputs.end()) {
				if (!entity.isEnabled()) 
					continue;
				entity.getEntity()->update(emptyInput);
			}
		}
	}

	void processWorldState()
	{
		for (auto& [id, entity] : entities) {
			if (entity.isEnabled()) 
			{
				entity.getHistory().serializeCurrentState();
			}

			// Only keep last two seconds of history, players lagging behind will get a full update
			entity.getHistory().clearHistoryUntil(this->frameNumber - 120);
		}
	}

	void sendOutgoingMessages() {
		if (this->frameNumber % config.networkUpdateRate == 0)
		{
			for (auto& [_, client] : clients) {
				auto clientNetwork = client->network.lock();
				if (!clientNetwork) 
					continue;

				OutgoingWorldStatePacket packet(
					client->getLastAcknowledgedFrame(), 
					this->frameNumber, 
					client->assignedEntityId,
					controlledEntityInputs,
					entities
				);
				clientNetwork->pushOutgoingPacket(packet);
			}
		}
	}

	void pruneEntities()
	{
		for (auto it = entities.begin(); it != entities.end();) {
			if (it->second.isReadyForDeletion()) {
				it = entities.erase(it);
			} else {
				++it;
			}
		}
	}

	ServerConfig config;
	
	Input emptyInput;

	std::unordered_map<int, BufferedInput<Input>> controlledEntityInputs;

	std::unordered_map<int, ServerEntity<EntityType, Entity, Input>> entities;

    std::unordered_map<int, std::shared_ptr<ClientConnection<EntityType, Entity, Input>>> clients;

};