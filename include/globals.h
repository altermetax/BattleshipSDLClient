#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_ttf.h>
#include "ship.h"

#define PROTOCOL_VERSION "1.0"
#define NUMBER_OF_SHIPS 5

extern char* nickname;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* gridSquareA;
extern SDL_Texture* gridSquareB;
extern SDL_Texture* mouseOverlay;
extern SDL_Texture* shipFront;
extern SDL_Texture* shipMiddle;
extern SDL_Texture* shipBack;
extern SDL_Texture* hitOverlay;
extern SDL_Texture* missedOverlay;
extern Ship* destroyer;
extern Ship* submarine;
extern Ship* cruiser;
extern Ship* battleship;
extern Ship* carrier;
extern Ship* globalShips[NUMBER_OF_SHIPS];
extern Ship* ships[NUMBER_OF_SHIPS];
extern TTF_Font* mainFont;
extern SDL_Texture** gridColLabels;
extern SDL_Texture** gridRowLabels;
extern int screenWidth;
extern int screenHeight;
extern int squareWidth;
extern int squareHeight;
extern int cols;
extern int rows;
extern unsigned char currentShip;
extern unsigned char currentScene;
extern SDL_Thread* networkThread;
extern char* serverAddress;
extern long int serverPort;
extern TCPsocket serverSocket;
extern SDLNet_SocketSet socketSet;
extern char opponentNickname[64];
typedef struct {
    char** map;
    SDL_mutex* mutex;
} Hitmap;
extern Hitmap* ownHitmap;
extern Hitmap* opponentHitmap;

// Game state flag data
#define STOP_RUNNING 0x1
#define MOUSE_LEFT_PRESSED 0x2
#define MOUSE_RIGHT_PRESSED 0x4
#define MOUSE_MIDDLE_PRESSED 0x8
#define MOUSE_WHEEL_UP 0x10
#define MOUSE_WHEEL_DOWN 0x20
#define END_SCENE 0x40