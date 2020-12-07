#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "globals.h"
#include "ship.h"

enum NetworkStateEnum {
    CONNECTING,
    WAITING_MATCH,
    PLACING_SHIPS,
    WAITING_SHIPS,
    OWN_TURN,
    WAITING_TURN,
    WON,
    LOST
};

enum HittingStateEnum {
    NO_HIT,
    HIT,
    HIT_SUNK,
    END
};

typedef struct {
    SDL_mutex* mutex;
    enum NetworkStateEnum state;
    enum HittingStateEnum hittingState;
    SDL_cond* clientSignal;
    char clientInfo;
    int x;
    int y;
} NetworkState;

extern NetworkState networkState;

void initNetwork();
int networkMain(void* data);
void lockMutex(SDL_mutex* m);
void setNetworkState(enum NetworkStateEnum s);
enum NetworkStateEnum getNetworkState();
void waitForServer(char* response, int maxResponseLength);
void waitForClientSignal();
void zeroClientSignal();
void runRequest(const char* message, char* response, int maxResponseLength);
char runHelloRequest();
void waitMatched();
void handleMatched(char** lineSavePtr);
char runReadyRequest();
char waitOpponentShipsPlaced();
void setHitmapField(Hitmap* hitmap, int x, int y, char value);
char getHitmapField(Hitmap* hitmap, int x, int y);
char handleOwnTurn();
void getOpponentActionCoords(char* secondLine, int* x, int* y);
char handleOpponentTurn();