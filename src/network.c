#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "load.h"
#include "network.h"
#include "globals.h"

NetworkState networkState;

// Initializes the network part of the application.
void initNetwork() {
    if(SDLNet_Init() != 0) {
        fprintf(stderr, "Error: couldn't initialize SDLNet:\n%s\n", SDLNet_GetError());
        exit(1);
    }
    IPaddress* ipAddress = malloc(sizeof(IPaddress));
    if(SDLNet_ResolveHost(ipAddress, serverAddress, serverPort) != 0) {
        fprintf(stderr, "Error: couldn't resolve host:\n%s\n", SDLNet_GetError());
        exit(1);
    }

    networkState.mutex = SDL_CreateMutex();
    if(!networkState.mutex) {
        fprintf(stderr, "Error: couldn't initialize network state mutex:\n%s\n", SDL_GetError());
        exit(1);
    }
    networkState.clientSignal = SDL_CreateCond();
    if(!networkState.clientSignal) {
        fprintf(stderr, "Error: couldn't initialize network state client signal:\n%s\n", SDL_GetError());
        exit(1);
    }
    networkState.state = CONNECTING;

    networkThread = SDL_CreateThread(networkMain, "network", ipAddress);
    if(!networkThread) {
        fprintf(stderr, "Error: couldn't create network thread:\n%s\n", SDL_GetError());
        exit(1);
    }
}

// Handles everything that has to do with communicating with the server. Should be run as a thread.
int networkMain(void* data) {
    IPaddress* ipAddress = (IPaddress*) data;

    socketSet = SDLNet_AllocSocketSet(1);
    if(!socketSet) {
        fprintf(stderr, "Error: couldn't allocate socketset for server:\n%s\n", SDL_GetError());
        exit(1);
    }

    serverSocket = SDLNet_TCP_Open(ipAddress);
    if(!serverSocket) {
        fprintf(stderr, "Error: couldn't connect to server:\n%s\n", SDLNet_GetError());
        exit(1);
    }
    free(ipAddress);

    SDLNet_TCP_AddSocket(socketSet, serverSocket);

    // Now run the hello request
    // If server responds wait_match, wait until it sends matched
    if(runHelloRequest() == 0) {
        waitMatched();
    }

    // Now wait for user to finish placing their ships, then send ready message to server;
    // Then, depending on outcome of request, wait for other client to send their ships
    waitForClientSignal();
    char turnStatus = runReadyRequest();
    zeroClientSignal();

    if(turnStatus == 0) { // Other client hasn't placed their ships yet
        turnStatus = waitOpponentShipsPlaced();
    }

    // Until server sends you_win or you_lose, run yourTurn and waitTurn depending on what turn
    while(turnStatus < 3) {
        if(turnStatus == 1) { // Own turn
            turnStatus = handleOwnTurn();
        }
        else if(turnStatus == 2) {
            turnStatus = handleOpponentTurn();
        }
    }

    SDLNet_TCP_Close(serverSocket);
}

// Locks the SDL_mutex passed to it.
void lockMutex(SDL_mutex* m) {
    if(SDL_LockMutex(m) != 0) {
        fprintf(stderr, "Error: couldn't lock network state mutex:\n%s\n", SDL_GetError());
        exit(1);
    }
}

// Sets the state field of the networkState variable thread-safely.
void setNetworkState(enum NetworkStateEnum s) {
    lockMutex(networkState.mutex);
    networkState.state = s;
    SDL_UnlockMutex(networkState.mutex);
}

// Gets the state field of the networkState variable thread-safely
enum NetworkStateEnum getNetworkState() {
    lockMutex(networkState.mutex);
    enum NetworkStateEnum s = networkState.state;
    SDL_UnlockMutex(networkState.mutex);
    return s;
}

// Waits until server sends something, and copies it into response.
// maxResponseLength is the size of the response string.
void waitForServer(char* response, int maxResponseLength) {
    while(!SDLNet_CheckSockets(socketSet, 1000)); // Need this to wait

    while(SDLNet_CheckSockets(socketSet, 0) && SDLNet_SocketReady(serverSocket)) { // Need this to receive until end
        // Receive 256 bytes at a time
        char buf[256] = "";
        int result = SDLNet_TCP_Recv(serverSocket, buf, 256);
        if(result < 0) {
            fprintf(stderr, "Error: couldn't receive from server:\n%s\n", SDLNet_GetError());
            exit(1);
        }
        else if(result == 0) {
            printf("Server closed connection.\n");
            exit(1);
        }

        if(maxResponseLength - strlen(response) < strlen(buf)) {
            fprintf(stderr, "Error: server's response to request too big.\n");
            exit(1);
        }

        strcat(response, buf);
    }
}

// Waits until networkState.clientInfo == 1 thread-safely.
void waitForClientSignal() {
    lockMutex(networkState.mutex);
    while(!networkState.clientInfo) {
        if(SDL_CondWait(networkState.clientSignal, networkState.mutex) != 0) {
            fprintf(stderr, "Error: couldn't wait for client signal correctly.\n");
            exit(1);
        }
    }
    SDL_UnlockMutex(networkState.mutex);
}

// Sets networkState.clientInfo back to 0 thread-safely
void zeroClientSignal() {
    lockMutex(networkState.mutex);
    networkState.clientInfo = 0;
    SDL_UnlockMutex(networkState.mutex);
}

// Runs the request contained in message and waits until the server responds, then copies the output into response
void runRequest(const char* message, char* response, int maxResponseLength) {
    if(SDLNet_TCP_Send(serverSocket, message, strlen(message) + 1) < strlen(message) + 1) {
        fprintf(stderr, "Error: couldn't send initial message to server:\n%s\n", SDLNet_GetError());
        exit(1);
    }
    waitForServer(response, maxResponseLength);
}

// Runs the hello request, to be sent as soon as connected to the server.
char runHelloRequest() { // Returns 0 if not matched yet, 1 if already matched
    char helloMessage[256];
    sprintf(helloMessage, "hello\r\nversion " PROTOCOL_VERSION "\r\nname %s\r\nrows %d\r\ncols %d\r\n\r\n", nickname, rows, cols);

    char serverResponse[512];
    runRequest(helloMessage, serverResponse, 512);

    char* lineSavePtr;
    char* header = strtok_r(serverResponse, "\r\n", &lineSavePtr);
    if(header == NULL) {
        fprintf(stderr, "Error: server's response after hello request only contained newlines.\n");
        exit(1);
    }

    if(strcmp(header, "wait_match") == 0) {
        setNetworkState(WAITING_MATCH);
        printf("Server responded to hello message with wait_match.\n");

        return 0;
    }

    else if(strcmp(header, "matched") == 0) {
        handleMatched(&lineSavePtr);
        printf("Server responded to hello message with matched. Opponent's nickname: %s.\n", opponentNickname);

        return 1;
    }

    else {
        fprintf(stderr, "Error: server returned following on response to hello message:\n%s\n", header);
    }
}

// Waits until the server sends a "matched" message.
void waitMatched() {
    char serverResponse[512] = "";
    waitForServer(serverResponse, 512);
    
    char* lineSavePtr;
    char* header = strtok_r(serverResponse, "\r\n", &lineSavePtr);
    
    if(strcmp(header, "matched") == 0) {
        handleMatched(&lineSavePtr);
        printf("Server sent matched. Opponent's nickname: %s.\n", opponentNickname);
    }
    else {
        fprintf(stderr, "Error: server returned following while waiting for match:\n%s\n", header);
        exit(1);
    }
}

// Handles a matched message and copies the opponent's nickname in the opponentNickname global variable.
void handleMatched(char** lineSavePtr) {
    char* line = strtok_r(NULL, "\r\n", lineSavePtr);
    if(line == NULL) {
        fprintf(stderr, "Error: server returned matched header, but didn't give opponent's nickname.\n");
        exit(1);
    }

    char* argSavePtr;
    char* param = strtok_r(line, " ", &argSavePtr);
    if(param == NULL) {
        fprintf(stderr, "Error: couldn't get opponent's nickname.\n");
        exit(1);
    }
    if(strcmp(param, "name") == 0) {
        char* name = strtok_r(NULL, " ", &argSavePtr);
        if(name == NULL) {
            fprintf(stderr, "Error: couldn't get opponent's nickname.\n");
            exit(1);
        }
        strcpy(opponentNickname, name);
    }
    else {
        fprintf(stderr, "Error: server returned invalid parameter on 'matched' response.\n");
        exit(1);
    }

    setNetworkState(PLACING_SHIPS);
}

// Runs the ready request; must be called when the ships have been completely placed.
// Returns 0 if server responds wait_ships, 1 if your_turn, 2 if wait_turn
char runReadyRequest() {
    char msg[65535];

    char* stringified = stringifyShips(ships, 5);
    if(stringified == NULL) {
        fprintf(stderr, "Error: trying to run ready request, but ships cannot be stringified.\n");
        exit(1);
    }
    sprintf(msg, "ready\r\n%s\r\n\r\n", stringified);
    free(stringified);

    printf("Running ready request\n");
    char serverResponse[512] = "";
    runRequest(msg, serverResponse, 512);

    char* lineSavePtr;
    char* header = strtok_r(serverResponse, "\r\n", &lineSavePtr);
    if(header == NULL) {
        fprintf(stderr, "Error: server's response after ready request only contained newlines.\n");
        exit(1);
    }

    if(strcmp(header, "wait_ships") == 0) {
        setNetworkState(WAITING_SHIPS);
        printf("Server responded to ready message with wait_ships.\n");

        return 0;
    }

    else if(strcmp(header, "your_turn") == 0) {
        setNetworkState(OWN_TURN);
        printf("Server reponded to ready message with your_turn.\n");
        return 1;
    }

    else if(strcmp(header, "wait_turn") == 0) {
        setNetworkState(WAITING_TURN);
        printf("Server responded to ready message with wait_turn.\n");
        return 2;
    }

    else {
        fprintf(stderr, "Error: server returned following on response to ready message:\n%s\n", header);
        exit(1);
    }
}

// Waits until the server sends your_turn or wait_turn (i. e. until the opponent has finished placing their ships)
char waitOpponentShipsPlaced() {
    char serverResponse[512] = "";
    waitForServer(serverResponse, 512);
    
    char* lineSavePtr;
    char* header = strtok_r(serverResponse, "\r\n", &lineSavePtr);
    
    if(strcmp(header, "your_turn") == 0) {
        setNetworkState(OWN_TURN);
        printf("Server sent your_turn.\n");
        return 1;
    }
    
    else if(strcmp(header, "wait_turn") == 0) {
        setNetworkState(WAITING_TURN);
        printf("Server sent wait_turn.\n");
        return 2;
    }

    else {
        fprintf(stderr, "Error: server returned following while waiting for match:\n%s\n", header);
        exit(1);
    }
}

// Sets a field in a Hitmap thread-safely
void setHitmapField(Hitmap* hitmap, int x, int y, char value) {
    lockMutex(hitmap->mutex);
    hitmap->map[y][x] = value;
    SDL_UnlockMutex(hitmap->mutex);
}

// Gets a field in a Hitmap thread-safely
char getHitmapField(Hitmap* hitmap, int x, int y) {
    lockMutex(hitmap->mutex);
    char value = hitmap->map[y][x];
    SDL_UnlockMutex(hitmap->mutex);
    return value;
}

// Handles player's turn
// Returns 2 if turn ended, 3 if win, 4 if lose
char handleOwnTurn() {
    waitForClientSignal();

    lockMutex(networkState.mutex);
    int x = networkState.x;
    int y = networkState.y;
    SDL_UnlockMutex(networkState.mutex);

    char msg[512];
    sprintf(msg, "attack\r\n%d %d\r\n\r\n", x, y);

    char serverResponse[512] = "";
    runRequest(msg, serverResponse, 512);

    char* lineSavePtr;
    char* header = strtok_r(serverResponse, "\r\n", &lineSavePtr);
    if(header == NULL) {
        fprintf(stderr, "Error: server response after attack only contained newlines.\n");
        exit(1);
    }

    char result;
    lockMutex(networkState.mutex);

    if(strcmp(header, "no_hit") == 0) {
        networkState.hittingState = NO_HIT;
        networkState.state = WAITING_TURN;
        setHitmapField(opponentHitmap, x, y, 1);
        result = 2;
    }

    else if(strcmp(header, "hit") == 0) {
        networkState.hittingState = HIT;
        networkState.state = WAITING_TURN;
        setHitmapField(opponentHitmap, x, y, 2);
        result = 2;
    }

    else if(strcmp(header, "hit_sunk") == 0) {
        networkState.hittingState = HIT_SUNK;
        networkState.state = WAITING_TURN;
        setHitmapField(opponentHitmap, x, y, 2);
        result = 2;
    }

    else if(strcmp(header, "you_win") == 0) {
        networkState.hittingState = END;
        networkState.state = WON;
        result = 3;
    }

    else if(strcmp(header, "you_lose") == 0) {
        networkState.hittingState = END;
        networkState.state = LOST;
        result = 4;
    }

    else {
        fprintf(stderr, "Error: server returned following response to attack message:\n%s\n", header);
        exit(1);
    }
    networkState.clientInfo = 0;
    SDL_UnlockMutex(networkState.mutex);
    return result;
}

// Interprets a string containing two numbers and writes them on two ints.
// Usually this is the second line of a no_hit, hit or hit_sunk message, which contains the coordinates.
void getOpponentActionCoords(char* secondLine, int* x, int* y) {
    char wrongFormatError[] = "Error: server sent badly formatted opponent action coordinates.\n";
    if(secondLine == NULL) {
        fprintf(stderr, wrongFormatError);
        exit(1);
    }
    if(sscanf(secondLine, "%d %d", x, y) != 2) {
        fprintf(stderr, wrongFormatError);
        exit(1);
    }
}

// Handles the turn of the opponent.
// Returns 1 on turn ended normally, 3 if won, 4 if lost.
char handleOpponentTurn() {
    char serverResponse[512] = "";
    waitForServer(serverResponse, 512);

    char* lineSavePtr;
    char* header = strtok_r(serverResponse, "\r\n", &lineSavePtr);
    if(header == NULL) {
        fprintf(stderr, "Error: server sent only newlines as opponent's action.\n");
        exit(1);
    }

    char* secondLine = strtok_r(NULL, "\r\n", &lineSavePtr);

    if(strcmp(header, "no_hit") == 0) {
        int x, y;
        getOpponentActionCoords(secondLine, &x, &y);

        lockMutex(networkState.mutex);
        networkState.hittingState = NO_HIT;
        networkState.state = OWN_TURN;
        setHitmapField(ownHitmap, x, y, 1);
        SDL_UnlockMutex(networkState.mutex);
        return 1;
    }

    else if(strcmp(header, "hit") == 0) {
        int x, y;
        getOpponentActionCoords(secondLine, &x, &y);

        lockMutex(networkState.mutex);
        networkState.hittingState = HIT;
        networkState.state = OWN_TURN;
        setHitmapField(ownHitmap, x, y, 2);
        SDL_UnlockMutex(networkState.mutex);
        return 1;
    }

    else if(strcmp(header, "hit_sunk") == 0) {
        int x, y;
        getOpponentActionCoords(secondLine, &x, &y);

        lockMutex(networkState.mutex);
        networkState.hittingState = HIT_SUNK;
        networkState.state = OWN_TURN;
        setHitmapField(ownHitmap, x, y, 2);
        SDL_UnlockMutex(networkState.mutex);
        return 1;
    }

    else if(strcmp(header, "you_win") == 0) {
        lockMutex(networkState.mutex);
        networkState.hittingState = END;
        networkState.state = WON;
        SDL_UnlockMutex(networkState.mutex);
        return 3;
    }

    else if(strcmp(header, "you_lose") == 0) {
        lockMutex(networkState.mutex);
        networkState.hittingState = END;
        networkState.state = LOST;
        SDL_UnlockMutex(networkState.mutex);
        return 4;
    }

    else {
        fprintf(stderr, "Error: server returned following response as opponent's action:\n%s\n", header);
        exit(1);
    }
}
