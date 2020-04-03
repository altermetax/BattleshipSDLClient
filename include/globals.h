#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_ttf.h>
#include "ship.h"

#define PROTOCOL_VERSION "1.0"
#define NUMBER_OF_SHIPS 5
#define NICKNAME "altermetaxxo"

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* gridSquareA;
SDL_Texture* gridSquareB;
SDL_Texture* mouseOverlay;
SDL_Texture* shipFront;
SDL_Texture* shipMiddle;
SDL_Texture* shipBack;
SDL_Texture* hitOverlay;
SDL_Texture* missedOverlay;
Ship* destroyer;
Ship* submarine;
Ship* cruiser;
Ship* battleship;
Ship* carrier;
Ship* globalShips[NUMBER_OF_SHIPS];
Ship* ships[NUMBER_OF_SHIPS];
TTF_Font* mainFont;
SDL_Texture** gridColLabels;
SDL_Texture** gridRowLabels;
int screenWidth;
int screenHeight;
int squareWidth;
int squareHeight;
int cols;
int rows;
unsigned char currentShip;
unsigned char currentScene;
SDL_Thread* networkThread;
char* serverAddress;
long int serverPort;
TCPsocket serverSocket;
SDLNet_SocketSet socketSet;
char opponentNickname[64];
typedef struct {
    char** map;
    SDL_mutex* mutex;
} Hitmap;
Hitmap* ownHitmap;
Hitmap* opponentHitmap;

// Game state flag data
#define STOP_RUNNING 0x1
#define MOUSE_LEFT_PRESSED 0x2
#define MOUSE_RIGHT_PRESSED 0x4
#define MOUSE_MIDDLE_PRESSED 0x8
#define MOUSE_WHEEL_UP 0x10
#define MOUSE_WHEEL_DOWN 0x20
#define END_SCENE 0x40