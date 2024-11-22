#pragma once

class GameInput {
public:
    GameInput();
    ~GameInput();

    void setLeftRight(int value);
    void setUpDown(int value);

    signed char leftRight;
    signed char upDown;
};