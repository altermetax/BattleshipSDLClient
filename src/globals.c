#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"

char* nickname;
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
Hitmap* ownHitmap;
Hitmap* opponentHitmap;