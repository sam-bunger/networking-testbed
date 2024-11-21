#pragma once

class GameInput {
public:
    GameInput();
    ~GameInput();

    void pressLeft();
    void releaseLeft(); 

    void pressRight();
    void releaseRight();
    
    void pressUp();
    void releaseUp();

    void pressDown();
    void releaseDown();

    signed char leftRight;
    signed char upDown;
};