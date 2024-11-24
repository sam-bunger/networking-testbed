#include "./GameInput.hpp"

GameInput::GameInput()
{
    leftRight = 0;
    upDown = 0;
    fire = 0;
}

GameInput::~GameInput()
{}

void GameInput::setLeftRight(int value)
{
    leftRight = value;
}

void GameInput::setUpDown(int value)
{
    upDown = value;
}

void GameInput::setFire(bool value)
{
    fire = value;
}
