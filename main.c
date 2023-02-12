#include <sys/types.h>    // This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>    // Not necessary but include it anyway
#include <stdlib.h>
#include <stdbool.h>
#include <psxetc.h>    // Includes some functions that controls the display
#include <psxgte.h>    // GTE header, not really used but libgpu.h depends on it
#include <psxgpu.h>    // GPU library header
#include <psxpad.h>
#include <psxapi.h>
#include "game.h"
#include "types.h"

#define OTLEN 8         // Ordering table length (recommended to set as a define
// so it can be changed easily)

DISPENV disp[2];
DRAWENV draw[2];
int db = 0;

u_long ot[2][OTLEN];    // Ordering table length
char pribuff[2][32768]; // Primitive buffer
char *nextpri;          // Next primitive pointer
u_char padbuff[2][34];  // Controller input buffers

typedef struct {
    const uint32_t *texture;
    uint32_t	mode;
    RECT		crect;
    RECT		prect;
} TextureInfo;

extern const uint32_t birdTexture[];
TextureInfo birdTextureInfo;
extern const uint32_t cloudTexture[];
TextureInfo cloudTextureInfo;
extern const uint32_t groundTexture[];
TextureInfo groundTextureInfo;
extern const uint32_t pipeTexture[];
TextureInfo pipeTextureInfo;

void display() {
    DrawSync(0);                // Wait for any graphics processing to finish

    VSync(0);                   // Wait for vertical retrace

    PutDispEnv(&disp[db]);      // Apply the DISPENV/DRAWENVs
    PutDrawEnv(&draw[db]);

    SetDispMask(1);             // Enable the display

    DrawOTag(ot[db] + OTLEN - 1);   // Draw the ordering table

    db = !db;                   // Swap buffers on every pass (alternates between 1 and 0)
    nextpri = pribuff[db];      // Reset next primitive pointer
}

u_short getTPageForTim(TextureInfo *tim) {
    return getTPage(tim->mode & 0x3, 0, tim->prect.x, tim->prect.y);
}

void loadTexture(const uint32_t *texture, TextureInfo *textureInfo) {
    TIM_IMAGE *tim;
    GetTimInfo( texture, tim ); /* Get TIM parameters */
    LoadImage(tim->prect, tim->paddr );		/* Upload texture to VRAM */
    DrawSync(0);
    if(tim->mode & 0x8 ) {
        LoadImage(tim->crect, tim->caddr );	/* Upload CLUT if present */
        DrawSync(0);
    }
    textureInfo->prect = *tim->prect;
    textureInfo->crect = *tim->crect;
    textureInfo->mode = tim->mode;
}

void loadTextures() {
    loadTexture(birdTexture, &birdTextureInfo);
    loadTexture(cloudTexture, &cloudTextureInfo);
    loadTexture(groundTexture, &groundTextureInfo);
    loadTexture(pipeTexture, &pipeTextureInfo);
}

void init(GameState *gameState) {
    // Reset graphics
    ResetGraph(0);

    // First buffer
    SetDefDispEnv(&disp[0], 0, 0, 320, 240);
    SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
    // Second buffer
    SetDefDispEnv(&disp[1], 0, 240, 320, 240);
    SetDefDrawEnv(&draw[1], 0, 0, 320, 240);

    draw[0].isbg = 1;               // Enable clear
    setRGB0(&draw[0], 118, 179, 222);
    draw[1].isbg = 1;
    setRGB0(&draw[1], 118, 179, 222);

    nextpri = pribuff[0];           // Set initial primitive pointer address

    InitPAD(padbuff[0], 34, padbuff[1], 34);
    StartPAD();
    ChangeClearPAD(1);

    loadTextures();

    // Load the internal font texture
    FntLoad(960, 0);
    // Create the text stream
    gameState->gameOverTextboxId = FntOpen(30, 48, 275, 100, 0, 100);
    gameState->scoreTextboxId = FntOpen(30, 225, 275, 100, 0, 100);
}

void drawRectangle(TILE *tile, int x, int y, int w, int h, int r, int g, int b, int orderLayer) {
    tile = (TILE *) nextpri;      // Cast next primitive

    setTile(tile);              // Initialize the primitive (very important)
    setXY0(tile, x, y);       // Set primitive (x,y) position
    setWH(tile, w, h);        // Set primitive size
    setRGB0(tile, r, g, b); // Set color yellow
    addPrim(ot[db]+orderLayer, tile);      // Add primitive to the ordering table

    nextpri += sizeof(TILE);    // Advance the next primitive pointer
}

void drawRectangleWithTexture(int x, int y, int w, int h, int orderLayer, TextureInfo *textureInfo) {
    SPRT *sprite = (SPRT*) nextpri;      // Cast next primitive
    DR_TPAGE *tpage;

    setSprt(sprite);              // Initialize the primitive (very important)
    setXY0(sprite, x, y);       // Set primitive (x,y) position
    setWH(sprite, w, h);        // Set primitive size
    setUV0(sprite, 0, 0); // Set texture coordinates
    setClut(sprite, 0, 0);
    setRGB0(sprite, 128, 128, 128); // set neutral color
    addPrim(ot[db]+orderLayer, sprite);      // Add primitive to the ordering table
    nextpri += sizeof(SPRT);    // Advance the next primitive pointer

    tpage = (DR_TPAGE *) nextpri;      // Cast next primitive
    setDrawTPage(tpage, 0, 0, getTPageForTim(textureInfo)); // Set the texture page
    addPrim(ot[db]+orderLayer, tpage);      // Add primitive to the ordering table
    nextpri += sizeof(DR_TPAGE);    // Advance the next primitive pointer
}

void drawPlayer(GameState *gameState) {
    int playerOrderingTableLevel = 5;
    SPRT *player = (SPRT *) nextpri;      // Cast next primitive
    DR_TPAGE *tpri;
    setSprt(player);              // Initialize the primitive (very important)
    setXY0(player, (int)gameState->x, (int)gameState->y);       // Set primitive (x,y) position
    setWH(player, PLAYER_SIZE, PLAYER_SIZE);        // Set primitive size
    setUV0(player, 0, 0); // Set texture coordinates
    setClut(player, 0, 0);
    setRGB0(player, 128, 128, 128);
    addPrim(ot[db]+playerOrderingTableLevel, player);      // Add primitive to the ordering table
    nextpri += sizeof(SPRT);    // Advance the next primitive pointer

    /* Sort a TPage primitive so the sprites will draw pixels from */
    /* the correct texture page in VRAM */
    tpri = (DR_TPAGE*)nextpri;
    setDrawTPage( tpri, 0, 0, getTPageForTim(&birdTextureInfo));
    addPrim( ot[db]+playerOrderingTableLevel, tpri );
    nextpri += sizeof(DR_TPAGE);
}

void drawWalls() {
//    TILE *wall;
    drawRectangleWithTexture(0, 220, 320, 20, 7, &groundTextureInfo);
//    drawRectangle(wall, 0, 220, 320, 20, 0, 0, 150, 7);
}

void drawPipes(GameState *gameState) {
    for(int i = 0; i < MAX_PIPES; i++) {
        if(gameState->pipes[i].isActive == false)
            continue;
        drawRectangleWithTexture((int)gameState->pipes[i].x, 0, 20, gameState->pipes[i].gapTopY, 6, &pipeTextureInfo);
        drawRectangleWithTexture((int)gameState->pipes[i].x, gameState->pipes[i].gapBottomY, 20, 240 - gameState->pipes[i].gapBottomY, 6, &pipeTextureInfo);
    }
}

void beforeGameLogic() {
    ClearOTagR(ot[db], OTLEN);  // Clear ordering table
}

void afterGameLogic() {
    display();
}

void initializeGameState(GameState *gameState) {
    gameState->score = 0;
    gameState->x = 50;
    gameState->y = 100;
    gameState->velocityX = 0;
    gameState->velocityY = 0;
    gameState->isGameOver = false;
    gameState->nextPipeIndex = 0;
    gameState->framesUntilPipeSpawn = generateNextPipeSpawnFrame();
    gameState->pad = (PADTYPE *) padbuff[0];

    for(int i = 0; i < MAX_PIPES; i++) {
        gameState->pipes[i].x = -20;
        gameState->pipes[i].gapTopY = 0;
        gameState->pipes[i].gapBottomY = 0;
        gameState->pipes[i].isActive = false;
        gameState->pipes[i].hasAwardedPoints = false;
    }
}

int main() {
    GameState gameState;
    initializeGameState(&gameState);

    init(&gameState);
    while (1) {
        beforeGameLogic();

        drawWalls();
        drawPlayer(&gameState);
        drawPipes(&gameState);

        processGameLogic(&gameState);

        if(gameState.isGameOver && isButtonPressed(gameState.pad, PAD_START)) {
            initializeGameState(&gameState);
        }

        afterGameLogic();
    }

    return 0;
}