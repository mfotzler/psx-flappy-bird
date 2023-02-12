#ifndef PSX_FLAPPY_BIRD_TYPES_H
#define PSX_FLAPPY_BIRD_TYPES_H

#define PLAYER_SIZE 20

typedef struct {
    TILE *tile;
    float x;
    float y;
    float velocityX;
    float velocityY;
    bool isGameOver;
    PADTYPE *pad;
} GameState;

#endif //PSX_FLAPPY_BIRD_TYPES_H