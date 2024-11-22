#pragma once

#include <deque>
#include <iostream>
#include <numeric>
#include "../packets/input/IncomingWorldStatePacket.hpp"

template<typename EntityType, class Entity, class Input>
class Reconciler {
public:
    Reconciler(int ticksPerSecond) 
        : ticksPerSecond(ticksPerSecond),
          accumulatedFrameAdjustment(0.0f),
          averageRoundTripTicks(0.0f),
          maxRoundTripTicks(0.0f),
          lastVerifiedFrame(-1) {}

    void reconcile(
        std::shared_ptr<IncomingWorldStatePacket<EntityType, Entity, Input>> latestWorldState,
        std::unordered_map<int, ClientEntity<EntityType, Entity, Input>>& entities,
        int controllerId,
        int& frameNumber,
        const std::map<int, uint64_t>& inputTimestamps,
        IPhysicsWorld<EntityType, Entity, Input>* world,
        INetworkWorldController<EntityType, Entity, Input>* controller
    ) {
        if (!latestWorldState) return;

        int verifiedFrame = latestWorldState->getServerFrame();
        if (verifiedFrame <= lastVerifiedFrame) return;
        lastVerifiedFrame = verifiedFrame;

        int predictedFrame = frameNumber;
        int latestClientFrame = reconcileEntities(latestWorldState, entities, controllerId, verifiedFrame);
        
        float roundTripTicks = calculateRoundTripTicks(latestClientFrame, inputTimestamps);
        updateRoundTripStatistics(roundTripTicks);
        
        int finalFrameAdjustment = calculateFrameAdjustment(predictedFrame, verifiedFrame);
        
        replayFrames(verifiedFrame, predictedFrame, finalFrameAdjustment, entities, world, controller, frameNumber);
    }

private:
    static const size_t ROUND_TRIP_HISTORY_SIZE = 20;

    int reconcileEntities(
        std::shared_ptr<IncomingWorldStatePacket<EntityType, Entity, Input>> latestWorldState,
        std::unordered_map<int, ClientEntity<EntityType, Entity, Input>>& entities,
        int controllerId,
        int verifiedFrame
    ) {
        int latestClientFrame = -1;

        for (auto& [id, entity] : entities) {
            if (latestWorldState->isEntityInPacket(id)) {
                WorldStateEntityHeader* header = latestWorldState->getEntityHeader(id);
                if (header->type != entity.getEntity()->getType()) {
                    continue;
                }

                if (latestWorldState->isControlledEntity(id)) {
                    WorldStateControlledEntityHeader<Input>* controlledHeader = 
                        latestWorldState->getControlledEntityHeader(id);
                    entity.timeline.serializeInputOntoTimeline(verifiedFrame, controlledHeader->input);
                    if (controlledHeader->id == controllerId) {
                        latestClientFrame = controlledHeader->latestClientFrame;
                    }
                }

                entity.getEntity()->deserialize(latestWorldState->getEntityData(id));
            } else {
                entity.timeline.deserializeFromTimeline(verifiedFrame, *entity.getEntity());
            }

            entity.timeline.clearBeforeFrame(verifiedFrame);
        }

        return latestClientFrame;
    }

    float calculateRoundTripTicks(int latestClientFrame, const std::map<int, uint64_t>& inputTimestamps) {
        if (latestClientFrame == -1) return 0.0f;

        uint64_t roundTripTime = getRoundTripTime(latestClientFrame, inputTimestamps);
        return ((float)roundTripTime / (1000.0f / (float)ticksPerSecond));
    }

    void updateRoundTripStatistics(float roundTripTicks) {
        maxRoundTripTicks = std::max(roundTripTicks, maxRoundTripTicks);
        
        roundTripTicksHistory.push_back(roundTripTicks);
        if (roundTripTicksHistory.size() > ROUND_TRIP_HISTORY_SIZE) {
            roundTripTicksHistory.pop_front();
        }
        
        if (!roundTripTicksHistory.empty()) {
            averageRoundTripTicks = std::accumulate(
                roundTripTicksHistory.begin(), 
                roundTripTicksHistory.end(), 
                0.0f
            ) / roundTripTicksHistory.size();
        }

        std::cout << "Round trip time: " << averageRoundTripTicks << " ticks " << maxRoundTripTicks << std::endl;
    }

    int calculateFrameAdjustment(int predictedFrame, int verifiedFrame) {
        float currentPredictionCount = (float)(predictedFrame - verifiedFrame);
        float frameAdjustment = averageRoundTripTicks - currentPredictionCount + 3.0f;
        frameAdjustment = std::abs(frameAdjustment) < 3.0 ? 0.0f : frameAdjustment;
        accumulatedFrameAdjustment += frameAdjustment;

        int finalFrameAdjustment = 0;
        if (std::abs(accumulatedFrameAdjustment) > 1.0) {
            finalFrameAdjustment = (int)accumulatedFrameAdjustment;
            accumulatedFrameAdjustment -= finalFrameAdjustment;
        }

        return finalFrameAdjustment;
    }

    void replayFrames(
        int verifiedFrame,
        int predictedFrame,
        int finalFrameAdjustment,
        std::unordered_map<int, ClientEntity<EntityType, Entity, Input>>& entities,
        IPhysicsWorld<EntityType, Entity, Input>* world,
        INetworkWorldController<EntityType, Entity, Input>* controller,
        int& frameNumber
    ) {
        frameNumber = verifiedFrame;
        
        for (int f = verifiedFrame + 1; f <= predictedFrame + finalFrameAdjustment; f++) {
            controller->tick();

            for (auto& [id, entity] : entities) {
                const Input& input = entity.timeline.getInput(f);
                entity.getEntity()->update(input);
            }

            world->physicsStepHook();
            
            for (auto& [id, entity] : entities) {
                entity.timeline.serializeDataOntoTimeline(f, *entity.getEntity());
            }
        }
    }

    uint64_t getRoundTripTime(int frame, const std::map<int, uint64_t>& inputTimestamps) {
        auto it = inputTimestamps.find(frame);
        if (it == inputTimestamps.end()) return 0;

        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count() - it->second;
    }

    int ticksPerSecond;
    float accumulatedFrameAdjustment;
    std::deque<float> roundTripTicksHistory;
    float averageRoundTripTicks;
    float maxRoundTripTicks;
    int lastVerifiedFrame;
}; 