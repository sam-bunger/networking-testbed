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
#include "../packets/output/OutgoingEntityLifecyclePacket.hpp"

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
		auto newEntity = ServerEntity<EntityType, Entity, Input>(this->world->createEntityHook(id, type, this));
		auto [it, inserted] = entities.insert({id, newEntity});
		entityLifecyclePackets.emplace_back(this->frameNumber, true, *it->second.getEntity());
		return (Entity*) it->second.getEntity();
	}

    virtual void destroyEntity(int id) override
	{
		auto it = entities.find(id);
		if (it == entities.end())
			return;

		it->second.setReadyForDeletion(true);
		entityLifecyclePackets.emplace_back(this->frameNumber, false, *it->second.getEntity());
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
				INetworkEntity<EntityType, Entity, Input>* e = entities.at(client.assignedEntityId).getEntity();
				e->update(input.input);
				controlledEntityInputs[e->getId()] = input;
			}
			
			++it;
		}

		// Update Non-Player Entities
		for (auto& [id, entity] : entities) {
			if (controlledEntityInputs.find(id) == controlledEntityInputs.end()) {
				entity.getEntity()->update(emptyInput);
			}
		}
	}

	void sendOutgoingMessages() {
		if (this->frameNumber % config.networkUpdateRate == 0)
		{
			OutgoingWorldStatePacket packet(this->frameNumber, controlledEntityInputs, entities);
			for (auto& [_, client] : clients) {
				auto clientNetwork = client->network.lock();
				if (!clientNetwork) 
					continue;
				clientNetwork->pushOutgoingPacket(packet);
			}
		}

		for (auto& packet : entityLifecyclePackets) {
			for (auto& [_, client] : clients) {
				auto clientNetwork = client->network.lock();
				if (!clientNetwork) 
					continue;
				clientNetwork->pushOutgoingPacket(packet);
			}
		}

		entityLifecyclePackets.clear();
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

	std::vector<OutgoingEntityLifecyclePacket<EntityType, Entity, Input>> entityLifecyclePackets;
};