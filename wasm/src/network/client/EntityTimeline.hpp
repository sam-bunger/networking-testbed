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
    
    void serializeOntoTimeline(int frameNumber, const Input &input, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        if (entities.find(frameNumber) == entities.end()) {
            entities[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        entities[frameNumber]->input = input;
        entity.serialize(entities[frameNumber]->data.get());
    }

    void serializeDataOntoTimeline(int frameNumber, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        if (entities.find(frameNumber) == entities.end()) {
            entities[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        entity.serialize(entities[frameNumber]->data.get());
    }

    void serializeInputOntoTimeline(int frameNumber, const Input &input)
    {
        if (entities.find(frameNumber) == entities.end()) {
            entities[frameNumber] = std::make_shared<TimelineElement<Input>>(dataSize);
        }
        entities[frameNumber]->input = input;
    }

    void deserializeFromTimeline(int frameNumber, INetworkEntity<EntityType, Entity, Input> &entity)
    {
        if (entities.find(frameNumber) == entities.end()) return;
        entity.deserialize(entities[frameNumber]->data.get());
    }

    void clearBeforeFrame(int frameNumber)
    {
        entities.erase(entities.begin(), entities.lower_bound(frameNumber));
    }

    const Input &getInput(int frameNumber)
    {
        if (entities.find(frameNumber) == entities.end()) return emptyInput;
        return entities[frameNumber]->input;
    }

    // Allows for interating over the timeline
    using iterator = typename std::map<int, std::shared_ptr<TimelineElement<Input>>>::iterator;
    using const_iterator = typename std::map<int, std::shared_ptr<TimelineElement<Input>>>::const_iterator;

    iterator begin() { return entities.begin(); }
    iterator end() { return entities.end(); }
    const_iterator begin() const { return entities.begin(); }
    const_iterator end() const { return entities.end(); }

private:
    int dataSize;
    std::map<int, std::shared_ptr<TimelineElement<Input>>> entities;
    const Input emptyInput;
};