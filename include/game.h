#pragma once
#include <SDL2/SDL.h>

void gameLoop();
Uint32 getTimeLeft(Uint32 nextTime);
char handleEvent(SDL_Event ev);
void setStatusBar(const char* text);
void drawGrid(int xOffset, int yOffset);
void drawGridCoords(int xOffset, int yOffset, int labelSet);
void drawHitmap(Hitmap* hitmap, int xOffset, int yOffset);
void handleAttack(int xOffset, int yOffset, int gridWidth, int gridHeight, char state);
void renderShip(Ship* ship, Uint8 alphaMod, int x, int y, int xOffset, int yOffset);
void convertMouseCoordsToGrid(int mouseX, int mouseY, int xOffset, int yOffset, int* gridX, int* gridY);
unsigned char handleShipPlacement(int xOffset, int yOffset, int gridWidth, int gridHeight, char state);
void drawShipPlacementOverlay(Ship* ship, int xOffset, int yOffset, int gridWidth, int gridHeight);
void drawPlacedShips(int xOffset, int yOffset);
Ship* checkCollisionWithPlacedShips(Ship* ship);