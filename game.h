#include "types.h"

#ifndef PSX_FLAPPY_BIRD_GAME_H
#define PSX_FLAPPY_BIRD_GAME_H

void processGameLogic(GameState *gameState);
bool isButtonPressed(PADTYPE *pad, PadButton button);
u_short generateNextPipeSpawnFrame();

#endif //PSX_FLAPPY_BIRD_GAME_H
