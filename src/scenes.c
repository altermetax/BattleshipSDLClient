#include <SDL2/SDL.h>
#include <stdio.h>
#include "globals.h"
#include "scenes.h"
#include "load.h"
#include "game.h"
#include "network.h"
#include "userstrings.h"

char runConnectingScene() {
	SDL_Event ev;
	char state = 0;
	while(SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	char status[256];
	sprintf(status, CONNECTING_MSG, serverAddress);
	setStatusBar(status);

	SDL_RenderPresent(renderer);

	return state;
}

char runMatchWaitingScene() {
	SDL_Event ev;
	char state = 0;
	while(SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	setStatusBar(CONNECTED_MSG);

	SDL_RenderPresent(renderer);

	return state;
}

char runShipPlacementScene() {
	SDL_Event ev;
	char state = 0;
	while (SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	int gridWidth = squareWidth * cols;
	int gridHeight = squareHeight * rows;

	drawGrid(squareWidth, squareHeight);
	drawGrid(gridWidth + 2 * squareWidth, squareHeight);
	drawGridCoords(0, 0, 0);
	drawGridCoords(gridWidth + squareWidth, 0, 1);
	drawPlacedShips(squareWidth, squareHeight);
	
	lockMutex(networkState.mutex);
	char ci = networkState.clientInfo;
	SDL_UnlockMutex(networkState.mutex);

	if(ci == 0) {
		unsigned char allPlaced = handleShipPlacement(squareWidth, squareHeight, gridWidth, gridHeight, state);
		if(allPlaced) {
			lockMutex(networkState.mutex);
			networkState.clientInfo = 1;
			SDL_CondSignal(networkState.clientSignal);
			SDL_UnlockMutex(networkState.mutex);
		}
	}
	
	SDL_RenderPresent(renderer);

	return state;
}

char runShipWaitingScene() {
	SDL_Event ev;
	char state = 0;
	while (SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	int gridWidth = squareWidth * cols;
	int gridHeight = squareHeight * rows;

	drawGrid(squareWidth, squareHeight);
	drawGrid(gridWidth + 2 * squareWidth, squareHeight);
	drawGridCoords(0, 0, 0);
	drawGridCoords(gridWidth + squareWidth, 0, 1);
	drawPlacedShips(squareWidth, squareHeight);
	setStatusBar(WAIT_SHIPS_MSG);
	
	SDL_RenderPresent(renderer);

	return state;
}

char runOwnTurnScene() {
	SDL_Event ev;
	char state = 0;
	while (SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	int gridWidth = squareWidth * cols;
	int gridHeight = squareHeight * rows;

	drawGrid(squareWidth, squareHeight);
	drawGrid(gridWidth + 2 * squareWidth, squareHeight);
	drawGridCoords(0, 0, 0);
	drawGridCoords(gridWidth + squareWidth, 0, 1);
	drawPlacedShips(squareWidth, squareHeight);
	drawHitmap(ownHitmap, squareWidth, squareHeight);
	drawHitmap(opponentHitmap, gridWidth + 2 * squareWidth, squareHeight);

	lockMutex(networkState.mutex);
	char ci = networkState.clientInfo;
	SDL_UnlockMutex(networkState.mutex);
	if(ci == 0) {
		handleAttack(gridWidth + 2 * squareWidth, squareHeight, gridWidth, gridHeight, state);
	}

	SDL_RenderPresent(renderer);

	return state;
}

char runTurnWaitingScene() {
	SDL_Event ev;
	char state = 0;
	while (SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	int gridWidth = squareWidth * cols;
	int gridHeight = squareHeight * rows;

	drawGrid(squareWidth, squareHeight);
	drawGrid(gridWidth + 2 * squareWidth, squareHeight);
	drawGridCoords(0, 0, 0);
	drawGridCoords(gridWidth + squareWidth, 0, 1);
	drawPlacedShips(squareWidth, squareHeight);
	drawHitmap(ownHitmap, squareWidth, squareHeight);
	drawHitmap(opponentHitmap, gridWidth + 2 * squareWidth, squareHeight);
	setStatusBar(WAIT_TURN_MSG);
	
	SDL_RenderPresent(renderer);

	return state;
}

char runWonScene() {
	SDL_Event ev;
	char state = 0;
	while(SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	setStatusBar(YOU_WIN_MSG);

	SDL_RenderPresent(renderer);

	return state;
}

char runLostScene() {
	SDL_Event ev;
	char state = 0;
	while(SDL_PollEvent(&ev)) {
		state |= handleEvent(ev);
	}

	SDL_RenderClear(renderer);

	setStatusBar(YOU_LOSE_MSG);

	SDL_RenderPresent(renderer);

	return state;
}
