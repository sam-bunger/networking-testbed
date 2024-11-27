#pragma once

#include <vector>
#include <memory>
#include <boost/pfr.hpp>
#include <iostream>

#include "./IEntityController.hpp"

struct DeltaState {
    static constexpr size_t MAX_FIELDS = 32;
    uint32_t changedFields;
    std::vector<uint8_t> data;

    int size() const {
        return sizeof(changedFields) + data.size();
    }

    bool isEmpty() const {
        return changedFields == 0 && data.empty();
    }
};

template<typename EntityType, class Entity, class Input> 
class INetworkEntity
{
public:
    INetworkEntity(int id, EntityType type, EntityController<EntityType, Entity>* controller) {
        this->id = id;
        this->type = type;
        this->controller = (EntityController<EntityType, Entity>*)controller;
    }

    virtual ~INetworkEntity() {}

    int getId() { return id; }
    EntityType getType() { return type; }
    EntityController<EntityType, Entity>* getController() { return controller; }

    virtual void addToWorld() = 0;
    virtual void removeFromWorld() = 0;

    virtual void update(const Input &input) = 0;
    virtual int serializeSize() = 0;
    virtual void serialize(void *buffer) = 0;
    virtual void deserialize(void *buffer) = 0;
    virtual void reset() = 0;

    virtual std::shared_ptr<DeltaState> createTypedDiff(void *oldState, void *newState) = 0;
    virtual void applyTypedDiff(void *oldState, DeltaState *deltaState, void *resultBuffer) = 0;

    template<std::size_t I = 0, typename State>
    typename std::enable_if<I == boost::pfr::tuple_size<State>::value>::type
    processFields(State*, State*, std::shared_ptr<DeltaState>&)
    {
        // Base case: do nothing
    }

    template<std::size_t I = 0, typename State>
    typename std::enable_if<I < boost::pfr::tuple_size<State>::value>::type
    processFields(State* oldState, State* newState, std::shared_ptr<DeltaState>& delta)
    {
        auto& field_old = boost::pfr::get<I>(*oldState);
        auto& field_new = boost::pfr::get<I>(*newState);

        if (field_old != field_new) {
            delta->changedFields |= (1u << I);

            // Calculate difference
            auto diff = field_new - field_old;

            // Append to data buffer
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&diff);
            delta->data.insert(delta->data.end(), bytes, bytes + sizeof(diff));
        }

        // Recurse to the next field
        processFields<I + 1>(oldState, newState, delta);
    }

    template<typename State>
    std::shared_ptr<DeltaState> createDiff(State* oldState, State* newState)
    {
        auto delta = std::make_shared<DeltaState>();
        delta->data.clear();
        delta->changedFields = 0;

        processFields(oldState, newState, delta);

        return delta;
    }

    template<typename State>
    void applyDiff(State* oldState, DeltaState* delta, State* resultBuffer)
    {
        // Start with old state
        *resultBuffer = *oldState;

        size_t dataOffset = 0;

        processFields(oldState, delta, resultBuffer, dataOffset);
    }

    template<std::size_t I = 0, typename State>
    typename std::enable_if<I == boost::pfr::tuple_size<State>::value>::type
    processFields(State*, DeltaState*, State*, size_t&)
    {
        // Base case: do nothing
    }

    template<std::size_t I = 0, typename State>
    typename std::enable_if<I < boost::pfr::tuple_size<State>::value>::type
    processFields(State* oldState, DeltaState* delta, State* resultBuffer, size_t& dataOffset)
    {
        if (delta->changedFields & (1u << I)) {
            auto& initialField = boost::pfr::get<I>(*oldState);
            auto& field = boost::pfr::get<I>(*resultBuffer);
            typedef typename std::remove_reference<decltype(field)>::type FieldType;

            // Get difference from data buffer
            const FieldType* diffPtr = reinterpret_cast<const FieldType*>(delta->data.data() + dataOffset);

            // Apply difference
            field = initialField + (*diffPtr);
            dataOffset += sizeof(FieldType);
        }

        // Recurse to the next field
        processFields<I + 1>(oldState, delta, resultBuffer, dataOffset);
    }

private:
    int id;
    EntityType type;
    EntityController<EntityType, Entity>* controller;
};