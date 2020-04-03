#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "ship.h"

// strtok_r is widely used in the program but not defined in mingw
#if defined(__MINGW32__) || defined(__MINGW64__)
char* strtok_r(char *str, const char *delim, char **nextp);
#endif
void init();
SDL_Texture* loadTexture(const char* path, int* width, int* height);
void renderCopy(SDL_Texture* texture, SDL_Rect* dstrect, double angle);
void setTextureAlphaMod(SDL_Texture* texture, Uint8 alphaMod);
TTF_Font* loadFont(const char* path, int ptsize);
SDL_Texture* getFontTexture(TTF_Font* font, const char* text, SDL_Color fgColor);
void loadGridCoordLabels();
char** allocateAndZeroMatrix(int sizeY, int sizeX);
void copyMatrix(char** dst, char** src, int sizeY, int sizeX);
void freeMatrix(char** matrix, int sizeY, int sizeX);
void loadShips();
Hitmap* initHitmap();
void destroy();
