#pragma once

#include <iostream>
#include <map>
#include "./DropRateCalculator.hpp"
#include "./BufferSizeCalculator.hpp"
#include "../../packets/input/IncomingPlayerInputPacket.hpp"

template<class Input>
class BufferedInput
{
public:
    BufferedInput() : latestClientFrame(0)
    {}

    BufferedInput(Input input, int latestClientFrame)
        : input(input), latestClientFrame(latestClientFrame)
    {}

    int latestClientFrame;
    Input input;
};

template<class Input>
class InputBuffer
{
public:
    InputBuffer() : lastRequestedFrameNumber(-1)
    { }

    void flush()
    {
        inputQueue.clear();
    }

    void addInputs(const std::vector<PlayerInputPacket<Input>> &inputs)
    {
        for (const auto& input : inputs)
        {
            inputQueue.emplace(input.frameNumber, input);
        }
    }

    BufferedInput<Input> getNextInput(int frame)
    {
        lastRequestedFrameNumber = frame;

        if (inputQueue.empty())
        {
            lastUsed.latestClientFrame = -1;
            return lastUsed;
        }

        // Find the largest frame in the queue
        int latestClientFrame = inputQueue.rbegin()->first;
        Input latestInput = inputQueue.rbegin()->second.input;

        // Remove all frames older than the requested frame
        auto staleIt = inputQueue.lower_bound(frame);
        if (staleIt != inputQueue.begin()) {
            inputQueue.erase(inputQueue.begin(), staleIt);
        }

        auto it = inputQueue.find(frame);
        
        if (it != inputQueue.end())
        {
            BufferedInput<Input> result(it->second.input, latestClientFrame);
            lastUsed = result;
            inputQueue.erase(it);  // Only remove the current frame since we already cleaned up old ones

            return result;
        }

        lastUsed.latestClientFrame = latestClientFrame;
        lastUsed.input = latestInput;

        return lastUsed;
    }

private:
    std::vector<Input> getInputList(const std::vector<PlayerInputPacket<Input>> &newInputs)
    {
        std::vector<Input> inputs;
        for (auto input : newInputs)
        {
            inputs.push_back(input.input);
        }
        return inputs;
    }

    BufferedInput<Input> lastUsed;
    std::map<int, PlayerInputPacket<Input>> inputQueue; 
    int lastRequestedFrameNumber;
};