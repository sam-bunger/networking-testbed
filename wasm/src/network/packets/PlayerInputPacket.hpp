#pragma once

#pragma pack(1)
struct PlayerInputPacketHeader 
{
    int acknowledgedFrame;
    char size;
};

template<class Input>
class PlayerInputPacket
{
public:
    PlayerInputPacket() : frameNumber(0) {}
    PlayerInputPacket(int frameNumber, Input input) : frameNumber(frameNumber), input(input) {};

    int getFrameNumber() { return frameNumber; }
    Input &getInput() { return input; }

    int frameNumber;
    Input input;
};
#pragma pack()