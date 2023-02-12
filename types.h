#ifndef PSX_FLAPPY_BIRD_TYPES_H
#define PSX_FLAPPY_BIRD_TYPES_H

#define PLAYER_SIZE 20
#define MAX_PIPES 4

typedef struct {
    float x;
    short gapTopY;
    short gapBottomY;
    bool isActive;
} PipePair;

typedef struct {
    TILE *tile;
    float x;
    float y;
    float velocityX;
    float velocityY;
    bool isGameOver;
    PipePair pipes[MAX_PIPES];
    uint16_t nextPipeIndex;
    uint16_t framesUntilPipeSpawn;
    PADTYPE *pad;
} GameState;

#endif //PSX_FLAPPY_BIRD_TYPES_H
