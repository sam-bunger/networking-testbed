#pragma once
#include <map>

#include "../INetworkEntity.hpp"

template<class Input>
class TimelineElement
{
public:
    TimelineElement(int dataSize) : data(std::make_unique<std::byte[]>(dataSize))
    { 
    }

    Input input;
    std::unique_ptr<std::byte[]> data;
};

template<typename EntityType, class Entity, class Input>
class EntityTimeline
{
public:
    EntityTimeline(int dataSize) : dataSize(dataSize) 
    {    }
    
    void copyOntoTimeline(int frameNumber, const Input &input, const void *data)
    {
        if (timeline.find(frameNumber) == timeline.end()) {
            timeline[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        timeline[frameNumber]->input = input;
        std::memcpy(timeline[frameNumber]->data.get(), data, dataSize);
    }

    void copyDataOntoTimeline(int frameNumber, const void *data)
    {
        if (timeline.find(frameNumber) == timeline.end()) {
            timeline[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        std::memcpy(timeline[frameNumber]->data.get(), data, dataSize);
    }

    void applyDeltasToTimeline(int initialFrame, int finalFrame, const Input &input, INetworkEntity<EntityType, Entity, Input> &entity, std::shared_ptr<DeltaState> deltas)
    {
        if (timeline.find(initialFrame) == timeline.end())
            return;

        if (timeline.find(finalFrame) == timeline.end()) 
        {
            timeline[finalFrame] = std::make_shared<TimelineElement<Input>>(dataSize);
        }

        timeline[finalFrame]->input = input;

        void *resultState = timeline[finalFrame]->data.get();
        void *initialState = timeline[initialFrame]->data.get();

        entity.applyTypedDiff(initialState, (DeltaState *)deltas.get(), resultState);
    }

    void serializeOntoTimeline(int frameNumber, const Input &input, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        if (timeline.find(frameNumber) == timeline.end()) {
            timeline[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        timeline[frameNumber]->input = input;
        entity.serialize(timeline[frameNumber]->data.get());
    }

    void serializeDataOntoTimeline(int frameNumber, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        if (timeline.find(frameNumber) == timeline.end()) 
        {
            timeline[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        entity.serialize(timeline[frameNumber]->data.get());
    }

    void deserializeFromTimeline(int frameNumber, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        if (timeline.find(frameNumber) == timeline.end()) return;
        entity.deserialize(timeline[frameNumber]->data.get());
    }

    void clearBeforeFrame(int frameNumber)
    {
        timeline.erase(timeline.begin(), timeline.lower_bound(frameNumber));
    }

    const Input &getInput(int frameNumber)
    {
        if (timeline.find(frameNumber) == timeline.end()) return emptyInput;
        return timeline[frameNumber]->input;
    }


private:
    int dataSize;
    std::map<int, std::shared_ptr<TimelineElement<Input>>> timeline;
    const Input emptyInput;
};