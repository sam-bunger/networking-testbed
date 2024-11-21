#include "./GameInput.hpp"

GameInput::GameInput()
{
    leftRight = 0;
    upDown = 0;
}

GameInput::~GameInput()
{}

void GameInput::pressLeft()
{
    if (leftRight == 0)
    {
        leftRight = -1;
    } else if (leftRight == 1)
    {
        leftRight = 0;
    }
}

void GameInput::pressRight()
{
    if (leftRight == 0)
    {
        leftRight = 1;
    } else if (leftRight == -1)
    {
        leftRight = 0;
    }
}

void GameInput::pressUp()
{
    if (upDown == 0)
    {
        upDown = -1;
    } else if (upDown == 1)
    {
        upDown = 0;
    }
}

void GameInput::pressDown()
{
    if (upDown == 0)
    {
        upDown = 1;
    } else if (upDown == -1)
    {
        upDown = 0;
    }
}

void GameInput::releaseLeft()
{
    if (leftRight == -1)
    {
        leftRight = 0;
    }
}

void GameInput::releaseRight()
{
    if (leftRight == 1)
    {
        leftRight = 0;
    }
}

void GameInput::releaseUp()
{
    if (upDown == -1)
    {
        upDown = 0;
    }
}

void GameInput::releaseDown()
{
    if (upDown == 1)
    {
        upDown = 0;
    }
}
