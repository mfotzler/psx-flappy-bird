#include <stdbool.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <stdlib.h>
#include "types.h"

#define FLAP_POWER 2

bool isColliding(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

bool isPlayerCollidingWithWall(int x, int y) {
    return isColliding(x, y, PLAYER_SIZE, PLAYER_SIZE, 0, 0, 320, 20) ||
           isColliding(x, y, PLAYER_SIZE, PLAYER_SIZE, 0, 220, 320, 20);
}

bool isController1Connected(GameState *gameState) {
    return gameState->pad->stat == 0;
}

bool isButtonPressed(PADTYPE *pad, PadButton button) {
    return !(pad->btn & button);
}

void updatePositionFromPad(GameState *gameState) {
    if (isController1Connected(gameState)) {
        gameState->x += gameState->velocityX;
        gameState->y += gameState->velocityY;
    }
}

bool isPlayerDead(GameState *gameState) {
    if (isPlayerCollidingWithWall(gameState->x, gameState->y))
        return true;

    return false;
}

void processGameLogic(GameState *gameState) {
    if (!gameState->isGameOver) {
        updatePositionFromPad(gameState);
    }

    if(isPlayerDead(gameState))
        gameState->isGameOver = true;

    if (gameState->isGameOver) {
        FntPrint(-1, "GAME OVER. PRESS START TO RESTART");
        FntFlush(-1);
    }
}