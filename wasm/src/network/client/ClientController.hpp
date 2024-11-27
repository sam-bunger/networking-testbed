#pragma once

#include <unordered_map>
#include <deque>
#include <iostream>
#include <vector>
#include <numeric>
#include "../INetwork.hpp"
#include "../INetworkWorldController.hpp"
#include "./ClientEntity.hpp"
#include "../packets/PlayerInputPacket.hpp"
#include "../packets/output/OutgoingPlayerInputPacket.hpp"
#include "../packets/input/IncomingWorldStatePacket.hpp"
struct ClientConfig 
{
    ClientConfig(int tps) : ticksPerSeconds(tps) {}
    int ticksPerSeconds;
};

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
        latestWorldState(NULL),
        accumulatedFrameAdjustment(0.0f),
        roundTripTicksHistory(),
        averageRoundTripTicks(0.0f),
        lastVerifiedFrame(-1)
	{ }


    virtual Entity* createEntity(EntityType type) override
	{
		int id = this->getNewEntityId(type);
		return (Entity*) createEntity(id, type, false, this->getFrameNumber());
	}

    virtual Entity* createEntity(int id, EntityType type) override
    {
        return (Entity*) createEntity(id, type, false, this->getFrameNumber());
    }

    virtual void destroyEntity(int id) override
	{
        destroyEntity(id, false, this->getFrameNumber());
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
		auto it = entities.find(id);
		return it != entities.end() && !it->second.isMarkedForDeletion();
	}

    void setControlledEntity(int id)
    {
        controllerId = id;
    }

    void tick(const Input &input)
    {
        processIncomingMessages();
        reconcile();

        INetworkWorldController<EntityType, Entity, Input>::tick();

        processLatestInputs(input);
        sendInputUpdate();

        updateEntites(input);
        this->world->physicsStepHook();
        serializeEntities(input);
    }

    const std::unordered_map<int, ClientEntity<EntityType, Entity, Input>>& getEntities()
	{
		return entities;
	}

    int getLastPredictedFrameCount()
    {
        return lastPredictedFrameCount;
    }

private:

    Entity* createEntity(int id, EntityType type, bool networkVerified, int frameCreated)
	{
		auto entityIt = entities.find(id);
		if (entityIt == entities.end()) 
        {
            // Create new entity if it doesn't exist
            auto newEntity = ClientEntity<EntityType, Entity, Input>(this->world->createEntityHook(id, type, this));
            auto [it, inserted] = entities.insert({id, newEntity});
            entityIt = it;
        }

        entityIt->second.setDeletionState(false);
        entityIt->second.resetFrameCreated(frameCreated);

		if (networkVerified) 
        {
            entityIt->second.networkCreated();
        }
        else
        {
            entityIt->second.getEntity()->reset();
        }

		return (Entity*)entityIt->second.getEntity();
	}

    void destroyEntity(int id, bool networkVerified, int frameDeleted)
    {
        auto it = entities.find(id);
		if (it == entities.end()) return;
        it->second.setDeletionState(true);
        it->second.resetFrameDeleted(frameDeleted);
        if (networkVerified) 
        {
            it->second.networkDeleted();
        }
    }

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
                       if (latestWorldState == nullptr || packet->getFinalServerFrame() > latestWorldState->getFinalServerFrame()) {
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

    /**
     * ROLLBACK RECONCILATION
     */
    void reconcile()
    {
        if (latestWorldState == NULL) return;

        int verifiedFrame = latestWorldState->getFinalServerFrame();
        if (verifiedFrame <= lastVerifiedFrame) return;
        lastVerifiedFrame = verifiedFrame;

        int latestClientFrame = latestWorldState->getLatestClientFrame();

        syncTimelineWithServerState(verifiedFrame);
        rollback(verifiedFrame);
        int finalFrame = determinePredictedFrame(verifiedFrame, latestClientFrame);
        rollforth(verifiedFrame, finalFrame);
        pruneObjectPool(verifiedFrame);
    }

    void syncTimelineWithServerState(int verifiedFrame)
    {
        int initialFrame = latestWorldState->getInitialServerFrame();
        for (auto& [id, header] : latestWorldState->getEntityHeaders()) {
            if (header->entityPacketType == WorldStateEntityPacketType::ENTITY_DELETED) 
            {
                destroyEntity(id, true, verifiedFrame);
            } 
            else if (header->entityPacketType == WorldStateEntityPacketType::ENTITY_FULL_RELOAD)
            {
                createEntity(id, (EntityType)header->type, true, verifiedFrame);

                ClientEntity<EntityType, Entity, Input>& entity = entities.at(id);

                if (latestWorldState->isControlledEntity(id))
                {
                    WorldStateControlledEntityHeader<Input>* controlledHeader = latestWorldState->getControlledEntityHeader(id);
                    entity.timeline.copyOntoTimeline(verifiedFrame, controlledHeader->input, latestWorldState->getEntityData(id));
                }
                else 
                {
                    entity.timeline.copyDataOntoTimeline(verifiedFrame, latestWorldState->getEntityData(id));
                }
            }
            else if (header->entityPacketType == WorldStateEntityPacketType::ENTITY_DELTA)
            {
                if (entities.find(id) == entities.end())
                    return;

                ClientEntity<EntityType, Entity, Input>& entity = entities.at(id);

                if (latestWorldState->isControlledEntity(id))
                {
                    WorldStateControlledEntityHeader<Input>* controlledHeader = latestWorldState->getControlledEntityHeader(id);
                    entity.timeline.applyDeltasToTimeline(initialFrame, verifiedFrame, controlledHeader->input, *entity.getEntity(), latestWorldState->deserializeDeltaState(id));
                }
                else 
                {
                    entity.timeline.applyDeltasToTimeline(initialFrame, verifiedFrame, Input(), *entity.getEntity(), latestWorldState->deserializeDeltaState(id));
                }
            }
        }
    }

    void rollback(int verifiedFrame)
    {
        // std::cout << "[ROLLBACK] Rolling back to frame: " << verifiedFrame << std::endl;
        for (auto& [id, entity] : entities) {
            if (entity.getFrameCreated() > verifiedFrame || (entity.getFrameDeleted() != -1 && entity.getFrameDeleted() <= verifiedFrame)) 
            {
                entity.setDeletionState(true);
            }
            else 
            {
                entity.setDeletionState(false);
                entity.timeline.deserializeFromTimeline(verifiedFrame, *entity.getEntity());
            }

            entity.timeline.clearBeforeFrame(latestWorldState->getInitialServerFrame());
        }
    }

    int determinePredictedFrame(int verifiedFrame, int latestClientFrame)
    {
        if (latestClientFrame != -1)
        {
            uint64_t roundTripTime = getRoundTripTime(latestClientFrame);

            float roundTripTicks = ((float)roundTripTime / (1000.0f / (float)config.ticksPerSeconds));

            // Update rolling average
            roundTripTicksHistory.push_back(roundTripTicks);
            if (roundTripTicksHistory.size() > ROUND_TRIP_HISTORY_SIZE) {
                roundTripTicksHistory.pop_front();
            }
            
            if (!roundTripTicksHistory.empty()) {
                averageRoundTripTicks = std::accumulate(roundTripTicksHistory.begin(), 
                                                    roundTripTicksHistory.end(), 
                                                    0.0f) / roundTripTicksHistory.size();
            }

            // Use exponential moving average for smoother RTT updates
            if (roundTripTicksHistory.empty()) {
                averageRoundTripTicks = roundTripTicks;
            } else {
                averageRoundTripTicks = averageRoundTripTicks * (1.0f - ADJUSTMENT_RATE) + 
                                    roundTripTicks * ADJUSTMENT_RATE;
            }
        }

        // Calculate target prediction frames based on RTT
        float targetPredictionFrames = std::max(
            MIN_PREDICTION_FRAMES,
            averageRoundTripTicks + BUFFER_FRAMES
        );

        int predictedFrame = this->frameNumber;
        float currentPredictionCount = (float)(predictedFrame - verifiedFrame);

        // Only adjust if we're significantly off from our target
        float predictionDelta = targetPredictionFrames - currentPredictionCount;
        if (std::abs(predictionDelta) > JITTER_THRESHOLD) {
            // Gradually move towards target to avoid sudden changes
            float adjustment = std::copysign(
                std::min(std::abs(predictionDelta) * ADJUSTMENT_RATE, 1.0f),
                predictionDelta
            );
            accumulatedFrameAdjustment += adjustment;
        }

        // Apply accumulated adjustments
        int finalFrameAdjustment = (int)accumulatedFrameAdjustment;
        accumulatedFrameAdjustment -= finalFrameAdjustment;

        // Determine final predicted frame
        int finalFrame = predictedFrame + finalFrameAdjustment;
        lastPredictedFrameCount = finalFrame - verifiedFrame;

        return finalFrame;
    }

    void rollforth(int verifiedFrame, int finalFrame)
    {
        this->frameNumber = verifiedFrame;
        int index = 1;
        for (int f = verifiedFrame + 1; f <= finalFrame; f++) {
            INetworkWorldController<EntityType, Entity, Input>::tick();
            // Reapply inputs
            for (auto& [id, entity] : entities) {
                if (entity.isMarkedForDeletion()) continue;

                if (id == controllerId) {
                    // Controlled user, inputs are not predicted
                    const Input &input = entity.timeline.getInput(f);
                    entity.getEntity()->update(input);
                }
                else 
                {
                    const Input &lastValidInput = entity.timeline.getInput(verifiedFrame);
                    entity.getEntity()->update(this->world->predictNextInput(lastValidInput, index));
                }
                
            }

            // Reapply physics
            this->world->physicsStepHook();
            
            // Store new state
            for (auto& [id, entity] : entities) {
                if (entity.isMarkedForDeletion()) continue;
                entity.timeline.serializeDataOntoTimeline(f, *entity.getEntity());
            }

            index++;
        }
    }

    void pruneObjectPool(int verifiedFrame)
    {
        for (auto it = entities.begin(); it != entities.end();)
        {
            bool isIncorrectPrediction = it->second.getNetworkState() == ClientEntityNetworkState::LOCAL_ONLY && it->second.getFrameCreated() < verifiedFrame;
            bool isMarkedForDeletion = it->second.getNetworkState() == ClientEntityNetworkState::NETWORK_DELETED && it->second.isMarkedForDeletion();
            if (isIncorrectPrediction || isMarkedForDeletion) {
                it = entities.erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * REGULAR UPDATE FUNCTIONALITY
     */
    void sendInputUpdate()
    {
        if (controllerId == -1) return;

        OutgoingPlayerInputPacket<Input> packet(lastVerifiedFrame, latestInputs);

        auto networkPtr = network.lock();
        if (networkPtr) 
        {
            networkPtr->pushOutgoingPacket(packet);
        }
    }

    void processLatestInputs(const Input &input)
    {
        latestInputs.push_back(PlayerInputPacket<Input>(this->getFrameNumber(), input));
        inputTimestamps[this->getFrameNumber()] = now();
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
            if (entity.isMarkedForDeletion()) continue;
            if (id != controllerId) {
                Input lastVerifiedInput = entity.timeline.getInput(lastVerifiedFrame);
                int framesAhead = this->getFrameNumber() - lastVerifiedFrame;
                entity.getEntity()->update(this->world->predictNextInput(lastVerifiedInput, framesAhead));
            }
        }
    }

    void serializeEntities(const Input &input)
    {
        for (auto& [id, entity] : entities) {
            if (controllerId != -1 && id == controllerId) 
            {
                entity.timeline.serializeOntoTimeline(this->frameNumber, input, *entity.getEntity());
            }
            else 
            {
                entity.timeline.serializeDataOntoTimeline(this->frameNumber, *entity.getEntity());
            }
        }
    }

    uint64_t getRoundTripTime(int frame)
    {
        // Remove everything older than the frame
        inputTimestamps.erase(inputTimestamps.begin(), inputTimestamps.lower_bound(frame));

        if (inputTimestamps.find(frame) == inputTimestamps.end()) {
            return -1;
        }

        return now() - inputTimestamps[frame];
    }

    uint64_t now()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }


    ClientConfig config;
    std::weak_ptr<INetwork> network;
    const Input emptyInput;
    std::unordered_map<int, ClientEntity<EntityType, Entity, Input>> entities;
    int controllerId;
    std::deque<PlayerInputPacket<Input>> latestInputs; 
    std::shared_ptr<IncomingWorldStatePacket<EntityType, Entity, Input>> latestWorldState;

    // Reconciliation
    std::map<int, uint64_t> inputTimestamps;
    float accumulatedFrameAdjustment;
    std::deque<float> roundTripTicksHistory;
    float averageRoundTripTicks;
    float lastPredictedFrameCount;
    int lastVerifiedFrame;

    static const size_t ROUND_TRIP_HISTORY_SIZE = 20; 
    static constexpr float JITTER_THRESHOLD = 0.5f;  // Frames of jitter we'll tolerate before adjusting
    static constexpr float ADJUSTMENT_RATE = 0.2f;   // How quickly we adjust to new conditions
    static constexpr float MIN_PREDICTION_FRAMES = 2.0f;  // Minimum frames we'll predict ahead
    static constexpr float BUFFER_FRAMES = 2.0f;     // Extra frames buffer for safety
};