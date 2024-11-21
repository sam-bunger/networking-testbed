#pragma once

#include "./DropRateCalculator.hpp"
#include "./BufferSizeCalculator.hpp"
#include "../../packets/input/IncomingPlayerInputPacket.hpp"
#include <iostream>

template<class Input>
class InputBuffer
{
public:
    InputBuffer() : bufferSizeCalculator(&realDropRate)
    {
        lastFrameProcessed = -1;
        realDropRate.setPacketDistance(3);
    }

    void flush()
    {
        inputQueue.clear();
        lastFrameProcessed = -1;
        realDropRate.reset();
    }

    void addInputs(const std::vector<PlayerInputPacket<Input>> &inputs)
    {
        int size = inputs.size();
        realDropRate.latestInput(inputs[size - 1].frameNumber);

        for (int i = 0; i < size; i++)
        {
            this->addSingleInput(inputs[i]);
        }
    }

    std::vector<PlayerInputPacket<Input>> getNextInputs()
    {
        std::vector<PlayerInputPacket<Input>> newInputs;

        // Reduce buffer until we find something that is <= than LFP
        while (inputQueue.size() > 1 && inputQueue.front().frameNumber <= lastFrameProcessed)
        {
            inputQueue.pop_front();
        }

        // Pull from the queue
        if (inputQueue.size() == 0)
        {
            // Buffer is empty!
            if (lastUsed.frameNumber == -1)
                return newInputs;
            lastUsed.frameNumber += 1;
            newInputs.push_back(lastUsed);
        }
        else if (inputQueue.size() > bufferSizeCalculator.getBufferSize())
        {
            // Buffer is full!
            newInputs.push_back(inputQueue.front());
            inputQueue.pop_front();
            newInputs.push_back(inputQueue.front());
            lastUsed = inputQueue.front();
            lastUsed.frameNumber -= 1;
            inputQueue.pop_front();
        }
        else
        {
            // Buffer is in a good state
            newInputs.push_back(inputQueue.front());
            lastUsed = inputQueue.front();
            inputQueue.pop_front();
        }

        lastFrameProcessed = newInputs.back().frameNumber;

        return newInputs;
    }

private:
    void addSingleInput(const PlayerInputPacket<Input> &input)
    {
        // If queue is empty or new input has higher frame number than the back, push to back
        if (inputQueue.empty() || input.frameNumber > inputQueue.back().frameNumber)
        {
            inputQueue.push_back(input);
            return;
        }

        // Find the appropriate position for the new value
        auto it = inputQueue.begin();
        while (it != inputQueue.end() && (*it).frameNumber < input.frameNumber)
        {
            ++it;
        }

        // If frame number already exists, replace it
        if (it != inputQueue.end() && (*it).frameNumber == input.frameNumber)
        {
            *it = input;
        }
        else
        {
            // Insert the new value at the correct position
            inputQueue.insert(it, input);
        }
    }

    std::vector<Input> getInputList(const std::vector<PlayerInputPacket<Input>> &newInputs)
    {
        std::vector<Input> inputs;
        for (auto input : newInputs)
        {
            inputs.push_back(input.input);
        }
        return inputs;
    }

    PlayerInputPacket<Input> lastUsed;

    std::list<PlayerInputPacket<Input>> inputQueue;
    int lastFrameProcessed;

    DropRateCalculator realDropRate;
    BufferSizeCalculator bufferSizeCalculator;
};