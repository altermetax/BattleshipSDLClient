#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "ship.h"
#include "network.h"
#include "load.h"

// strtok_r is widely used in the program but not defined in mingw
#if defined(__MINGW32__) || defined(__MINGW64__)
char* strtok_r(char *str, const char *delim, char **nextp) {
    char *ret;
    if (str == NULL) {
        str = *nextp;
    }
    str += strspn(str, delim);
    if (*str == '\0') {
        return NULL;
    }
    ret = str;
    str += strcspn(str, delim);
    if (*str) {
        *str++ = '\0';
    }
    *nextp = str;
    return ret;
}
#endif

void init() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Error: couldn't initialize SDL:\n%s", SDL_GetError());
		exit(1);
	}
	initNetwork();
	if(TTF_Init() < 0) {
		printf("Error: couldn't initialize SDL_ttf:\n%s", TTF_GetError());
		exit(1);
	}

	screenWidth = squareWidth * (2 * cols + 2);
	screenHeight = squareHeight * (rows + 1) + 50;

	window = SDL_CreateWindow("Battleship", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	if(window == NULL) {
		printf("Error: couldn't create SDL window:\n%s", SDL_GetError());
		exit(1);
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if(renderer == NULL) {
		printf("Error: couldn't create SDL renderer:\n%s", SDL_GetError());
		exit(1);
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	gridSquareA = loadTexture("resources/grid_square_a.bmp", NULL, NULL);
	gridSquareB = loadTexture("resources/grid_square_b.bmp", NULL, NULL);
	mouseOverlay = loadTexture("resources/mouse_overlay.bmp", NULL, NULL);
	shipFront = loadTexture("resources/ship_front.bmp", NULL, NULL);
	shipMiddle = loadTexture("resources/ship_middle.bmp", NULL, NULL);
	shipBack = loadTexture("resources/ship_back.bmp", NULL, NULL);
	hitOverlay = loadTexture("resources/hit_overlay.bmp", NULL, NULL);
	missedOverlay = loadTexture("resources/missed_overlay.bmp", NULL, NULL);

	mainFont = loadFont("resources/november.ttf", 30);
	loadGridCoordLabels();
	loadShips();
	ownHitmap = initHitmap();
	opponentHitmap = initHitmap();
	memset(ships, 0, sizeof ships);
	currentShip = 0;
}

SDL_Texture* loadTexture(const char* path, int* width, int* height) {
	SDL_Surface* surface = SDL_LoadBMP(path);
	if(surface == NULL) {
		printf("Error: couldn't load texture %s:\n%s", path, SDL_GetError());
		exit(1);
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if(texture == NULL) {
		printf("Error: couldn't convert surface to texture:\n%s", SDL_GetError());
		exit(1);
	}
	SDL_FreeSurface(surface);
	if(width != NULL && height != NULL) {
		SDL_QueryTexture(texture, NULL, NULL, width, height);
	}
	return texture;
}

void renderCopy(SDL_Texture* texture, SDL_Rect* dstrect, double angle) {
	if(SDL_RenderCopyEx(renderer, texture, NULL, dstrect, angle, NULL, SDL_FLIP_NONE) < 0) {
		printf("Error: couldn't copy texture to window:\n%s", SDL_GetError());
		exit(1);
	}
}

void setTextureAlphaMod(SDL_Texture* texture, Uint8 alphaMod) {
	if(SDL_SetTextureAlphaMod(texture, alphaMod) < 0) {
		printf("Error: couldn't set alpha mod on texture:\n%s", SDL_GetError());
		exit(1);
	}
}

TTF_Font* loadFont(const char* path, int ptsize) {
	TTF_Font* font = TTF_OpenFont(path, ptsize);
	if(font == NULL) {
		printf("Error: couldn't load font: \n%s", SDL_GetError());
		exit(1);
	}
	return font;
}

SDL_Texture* getFontTexture(TTF_Font* font, const char* text, SDL_Color fgColor) {
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, fgColor);
	if(surface == NULL) {
		printf("Error: couldn't make font texture:\n%s", TTF_GetError());
		exit(1);
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if(texture == NULL) {
		printf("Error: couldn't convert font surface to texture:\n%s", SDL_GetError());
		exit(1);
	}
	SDL_FreeSurface(surface);
	return texture;
}

void loadGridCoordLabels() {
	size_t gridColLabelsSize = 2 * (size_t)cols * sizeof(SDL_Texture*);
	if(gridColLabelsSize < 16) gridColLabelsSize = 16; // Must be at least 16 to avoid buffer overflow when writing
	size_t gridRowLabelsSize = 2 * (size_t)rows * sizeof(SDL_Texture*);
	if(gridRowLabelsSize < 16) gridRowLabelsSize = 16;
	gridColLabels = malloc(gridColLabelsSize);
	if(gridColLabels == NULL) {
		printf("Error: couldn't allocate memory (%zu bytes) for column coordinate labels.\n", gridColLabelsSize);
		exit(1);
	}
	gridRowLabels = malloc(gridRowLabelsSize);
	if(gridRowLabels == NULL) {
		printf("Error: couldn't allocate memory (%zu bytes) for row coordinate labels.\n", gridRowLabelsSize);
		exit(1);
	}
	// Write all column labels
	for(int set = 0; set < 2; set++) {
		for(int n = 0; n < cols; n++) {
			char label[3];
			label[0] = 65 + n;
			label[1] = '\0';
			SDL_Color c;
			if(set == 0) {
				c.r = 0;
				c.g = 255;
				c.b = 217;
				c.a = 255;
			}
			else {
				c.r = 255;
				c.g = 136;
				c.b = 0;
				c.a = 255;
			}
			gridColLabels[set * cols + n] = getFontTexture(mainFont, label, c);
		}
	}
	// Write all row labels
	for(int set = 0; set < 2; set++) {
		for(int n = 0; n < rows; n++) {
			char label[3];
			snprintf(label, sizeof(label), "%d", n + 1);
			SDL_Color c;
			if(set == 0) {
				c.r = 0;
				c.g = 255;
				c.b = 217;
				c.a = 255;
			}
			else {
				c.r = 255;
				c.g = 136;
				c.b = 0;
				c.a = 255;
			}
			gridRowLabels[set * rows + n] = getFontTexture(mainFont, label, c);
		}
	}
}

char** allocateAndZeroMatrix(int sizeY, int sizeX) {
	size_t byteSizeY = sizeY * sizeof(char*);
	size_t byteSizeX = sizeX * sizeof(char);
	if(byteSizeY < 16) byteSizeY = 16;
	if(byteSizeX < 16) byteSizeX = 16;
	char** matrix = malloc(byteSizeY);
	if(matrix == NULL) {
		printf("Error: couldn't allocate memory for matrix.\n");
		exit(1);
	}
	for(int y = 0; y < sizeY; y++) {
		matrix[y] = malloc(byteSizeX);
		if(matrix[y] == NULL) {
			printf("Error: couldn't allocate memory for row of a matrix.\n");
			exit(1);
		}
		for(int x = 0; x < sizeX; x++) {
			matrix[y][x] = 0;
		}
	}
	return matrix;
}

void copyMatrix(char** dst, char** src, int sizeY, int sizeX) {
	for(int y = 0; y < sizeY; y++) {
		memcpy(dst[y], src[y], sizeX * sizeof(char));
	}
}

void freeMatrix(char** matrix, int sizeY, int sizeX) {
	for(int y = 0; y < sizeY; y++) {
		free(matrix[y]);
	}
	free(matrix);
}

void loadShips() {
	destroyer = makeShip("destroyer", 0, 5, 5);
	submarine = makeShip("submarine", 1, 5, 5);
	cruiser = makeShip("cruiser", 2, 5, 5);
	battleship = makeShip("battleship", 3, 5, 5);
	carrier = makeShip("carrier", 4, 5, 5);

	memcpy(destroyer->matrix[0], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));
	memcpy(destroyer->matrix[1], (char[5]) { 0, 0, 'F', 0, 0 }, 5 * sizeof(char));
	memcpy(destroyer->matrix[2], (char[5]) { 0, 0, 'B', 0, 0 }, 5 * sizeof(char));
	memcpy(destroyer->matrix[3], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));
	memcpy(destroyer->matrix[4], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));

	memcpy(submarine->matrix[0], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));
	memcpy(submarine->matrix[1], (char[5]) { 0, 0, 'F', 0, 0 }, 5 * sizeof(char));
	memcpy(submarine->matrix[2], (char[5]) { 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(submarine->matrix[3], (char[5]) { 0, 0, 'B', 0, 0 }, 5 * sizeof(char));
	memcpy(submarine->matrix[4], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));

	memcpy(cruiser->matrix[0], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));
	memcpy(cruiser->matrix[1], (char[5]) { 0, 0, 'F', 0, 0 }, 5 * sizeof(char));
	memcpy(cruiser->matrix[2], (char[5]) { 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(cruiser->matrix[3], (char[5]) { 0, 0, 'B', 0, 0 }, 5 * sizeof(char));
	memcpy(cruiser->matrix[4], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));

	memcpy(battleship->matrix[0], (char[5]) { 0, 0, 'F', 0, 0 }, 5 * sizeof(char));
	memcpy(battleship->matrix[1], (char[5]) { 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(battleship->matrix[2], (char[5]) { 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(battleship->matrix[3], (char[5]) { 0, 0, 'B', 0, 0 }, 5 * sizeof(char));
	memcpy(battleship->matrix[4], (char[5]) { 0, 0, 0, 0, 0 }, 5 * sizeof(char));

	memcpy(carrier->matrix[0], (char[5]){ 0, 0, 'F', 0, 0 }, 5 * sizeof(char));
	memcpy(carrier->matrix[1], (char[5]){ 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(carrier->matrix[2], (char[5]){ 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(carrier->matrix[3], (char[5]){ 0, 0, 'M', 0, 0 }, 5 * sizeof(char));
	memcpy(carrier->matrix[4], (char[5]){ 0, 0, 'B', 0, 0 }, 5 * sizeof(char));

	globalShips[0] = destroyer;
	globalShips[1] = submarine;
	globalShips[2] = cruiser;
	globalShips[3] = battleship;
	globalShips[4] = carrier;
}

Hitmap* initHitmap() {
	Hitmap* hitmap = malloc(sizeof(Hitmap));
	hitmap->mutex = SDL_CreateMutex();
	if(hitmap->mutex == NULL) {
		fprintf(stderr, "Error: couldn't make mutex for hitmap:\n%s\n", SDL_GetError());
		exit(1);
	}
	hitmap->map = allocateAndZeroMatrix(rows, cols);
	return hitmap;
}

void destroy() {
	SDL_DestroyTexture(gridSquareA);
	SDL_DestroyTexture(gridSquareB);
	SDL_DestroyTexture(mouseOverlay);
	SDL_DestroyTexture(shipFront);
	SDL_DestroyTexture(shipMiddle);
	SDL_DestroyTexture(shipBack);
	SDL_DestroyTexture(hitOverlay);
	SDL_DestroyTexture(missedOverlay);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
