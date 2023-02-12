#include <stdbool.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

#define FLAP_POWER 3
#define GRAVITY_POWER 0.5
#define PIPE_FREQUENCY 100
#define GAP_SIZE 80
#define SCROLL_SPEED 3

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

void applyControllerActions(GameState *gameState) {
    if (isController1Connected(gameState)) {
        if (isButtonPressed(gameState->pad, PAD_CROSS))
            gameState->velocityY = -FLAP_POWER;
    }
}

bool isPlayerCollidingWithPipes(GameState *gameState) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if(gameState->pipes[i].isActive == false)
            continue;

        if(isColliding(gameState->x, gameState->y, PLAYER_SIZE, PLAYER_SIZE, gameState->pipes[i].x, 0, 20, gameState->pipes[i].gapTopY))
            return true;

        if(isColliding(gameState->x, gameState->y, PLAYER_SIZE, PLAYER_SIZE, gameState->pipes[i].x, gameState->pipes[i].gapBottomY, 20, 240 - gameState->pipes[i].gapBottomY))
            return true;
    }

    return false;
}

bool isPlayerDead(GameState *gameState) {
    if (isPlayerCollidingWithWall(gameState->x, gameState->y))
        return true;

    if(isPlayerCollidingWithPipes(gameState))
        return true;

    return false;
}

void applyGravity(GameState *gameState) {
    gameState->velocityY += GRAVITY_POWER;
}

void updatePlayerPosition(GameState *gameState) {
    gameState->x += gameState->velocityX;
    gameState->y += gameState->velocityY;
}

uint16_t generateNextPipeSpawnFrame() {
    int randomOffsetRange = PIPE_FREQUENCY / 4;
    return PIPE_FREQUENCY + (rand() % randomOffsetRange) - (randomOffsetRange / 2);
}

void spawnPipe(GameState *gameState) {
    gameState->pipes[gameState->nextPipeIndex].isActive = true;
    gameState->pipes[gameState->nextPipeIndex].hasAwardedPoints = false;
    gameState->pipes[gameState->nextPipeIndex].x = 320.0;
    uint16_t gapTopY = rand() % (200 - GAP_SIZE);
    gameState->pipes[gameState->nextPipeIndex].gapTopY = gapTopY;
    gameState->pipes[gameState->nextPipeIndex].gapBottomY = gapTopY + GAP_SIZE;

    gameState->nextPipeIndex = (gameState->nextPipeIndex + 1) % MAX_PIPES;
}

void processPipeDespawn(GameState *gameState) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if(gameState->pipes[i].x < -20)
            gameState->pipes[i].isActive = false;
    }
}

void checkOnPipeSpawn(GameState *gameState) {
    if(gameState->framesUntilPipeSpawn == 0) {
        gameState->framesUntilPipeSpawn = generateNextPipeSpawnFrame();
        spawnPipe(gameState);
    } else {
        gameState->framesUntilPipeSpawn--;
    }
}

void scrollPipes(GameState *gameState) {
    for (int i = 0; i < MAX_PIPES; i++) {
        gameState->pipes[i].x -= SCROLL_SPEED;
    }
}

void updatePipes(GameState *gameState) {
    checkOnPipeSpawn(gameState);
    processPipeDespawn(gameState);
    scrollPipes(gameState);
}

void checkForScore(GameState *gameState) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if(gameState->pipes[i].isActive && gameState->pipes[i].x < gameState->x
                && !gameState->pipes[i].hasAwardedPoints) {
            gameState->pipes[i].hasAwardedPoints = true;
            gameState->score++;
        }
    }
}

void processGameLogic(GameState *gameState) {
    if (!gameState->isGameOver) {
        updatePipes(gameState);
        checkForScore(gameState);
        applyGravity(gameState);
        applyControllerActions(gameState);
        updatePlayerPosition(gameState);
    }

    if(isPlayerDead(gameState))
        gameState->isGameOver = true;

    FntPrint(gameState->scoreTextboxId, "SCORE: %d", gameState->score);
    FntFlush(gameState->scoreTextboxId);

    if (gameState->isGameOver) {
        FntPrint(gameState->gameOverTextboxId, "GAME OVER. PRESS START TO RESTART");
        FntFlush(gameState->gameOverTextboxId);
    }
}