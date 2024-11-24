#pragma once

class GameInput {
public:
    GameInput();
    ~GameInput();

    void setLeftRight(int value);
    void setUpDown(int value);
    void setFire(bool value);

    signed char leftRight;
    signed char upDown;
    bool fire;
};