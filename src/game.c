#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "load.h"
#include "game.h"
#include "ship.h"
#include "scenes.h"
#include "globals.h"
#include "userstrings.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>

void gameLoop() {
	int fps = 30;
	Uint32 tickTime = (Uint32) (1000 / fps);
	Uint32 nextTime = 0;
	enum NetworkStateEnum ns;
	char state = 0;

	while(!(state & STOP_RUNNING)) {
		ns = getNetworkState();
		switch(ns) {
		case CONNECTING:
			state = runConnectingScene();
			break;
		case WAITING_MATCH:
			state = runMatchWaitingScene();
			break;
		case PLACING_SHIPS:
			state = runShipPlacementScene();
			break;
		case WAITING_SHIPS:
			state = runShipWaitingScene();
			break;
		case OWN_TURN:
			state = runOwnTurnScene();
			break;
		case WAITING_TURN:
			state = runTurnWaitingScene();
			break;
		case WON:
			state = runWonScene();
			break;
		case LOST:
			state = runLostScene();
			break;
		default:
			printf("Error: invalid network state.\n");
			exit(1);
		}
		if(state & END_SCENE) currentScene++;
		SDL_Delay(getTimeLeft(nextTime));
		nextTime += tickTime;
	}
}

Uint32 getTimeLeft(Uint32 nextTime) {
	Uint32 now = SDL_GetTicks();
	if(nextTime <= now) {
		return 0;
	}
	else {
		return nextTime - now;
	}
}

char handleEvent(SDL_Event ev) {
	if(ev.type == SDL_QUIT) {
		return STOP_RUNNING;
	}
	else if(ev.type == SDL_MOUSEBUTTONDOWN) {
		if(ev.button.button == SDL_BUTTON_LEFT) {
			return MOUSE_LEFT_PRESSED;
		}
		else if(ev.button.button == SDL_BUTTON_RIGHT) {
			return MOUSE_RIGHT_PRESSED;
		}
		else if(ev.button.button == SDL_BUTTON_MIDDLE) {
			return MOUSE_MIDDLE_PRESSED;
		}
	}
	else if(ev.type == SDL_MOUSEWHEEL) {
		if(ev.wheel.y > 0) {
			return MOUSE_WHEEL_UP;
		}
		else if(ev.wheel.y < 0) {
			return MOUSE_WHEEL_DOWN;
		}
	}
	else if(ev.type == SDL_KEYDOWN) {
		switch(ev.key.keysym.sym) {
		default:
			return 0;
		}
	}
	return 0;
}

void convertMouseCoordsToGrid(int mouseX, int mouseY, int xOffset, int yOffset, int* gridX, int* gridY) {
	*gridX = ((mouseX - xOffset) / squareWidth);
	*gridY = ((mouseY - yOffset) / squareHeight);
}

void setStatusBar(const char* text) {
	int fontWidth, fontHeight;
	SDL_Color c = {255, 255, 255, 255};
	SDL_Texture* texture = getFontTexture(mainFont, text, c);

	if(SDL_QueryTexture(texture, NULL, NULL, &fontWidth, &fontHeight) < 0) {
		printf("Error: couldn't get font width and height:\n%s", SDL_GetError());
		exit(1);
	}

	int usedWidth = fontWidth, usedHeight = 30, usedY = squareHeight * rows + squareHeight + 10;

	if(usedWidth > screenWidth) {
		usedWidth = screenWidth - 20;
		usedHeight = 30 * usedWidth / fontWidth;
		usedY = squareHeight * rows + squareHeight + ((squareHeight - usedHeight) / 2);
	}
	SDL_Rect r = {.x = 10, .y = usedY, .w = usedWidth, .h = usedHeight};
	renderCopy(texture, &r, 0);
	SDL_DestroyTexture(texture);
}

void drawGrid(int xOffset, int yOffset) {
	for(int x = 0; x < cols; x++) {
		for(int y = 0; y < rows; y++) {
			SDL_Texture* currentSquare;
			if(((x % 2) != 0) ^ ((y % 2) != 0)) currentSquare = gridSquareB;
			else currentSquare = gridSquareA;
			SDL_Rect r = { .x = xOffset + x * squareWidth, .y = yOffset + y * squareHeight, .w = squareWidth, .h = squareHeight };
			renderCopy(currentSquare, &r, 0);
		}
	}
}

void drawGridCoords(int xOffset, int yOffset, int labelSet) {
	for(int x = 1; x <= cols; x++) {
		SDL_Texture* texture = gridColLabels[cols * labelSet + x - 1];
		int fontWidth, fontHeight;
		if(SDL_QueryTexture(texture, NULL, NULL, &fontWidth, &fontHeight) < 0) {
			printf("Error: couldn't get font width and height:\n%s", SDL_GetError());
			exit(1);
		}
		SDL_Rect r = { .x = xOffset + x * squareWidth + squareWidth / 2 - fontWidth / 2,.y = yOffset + squareHeight / 2 - fontHeight / 2,.w = fontWidth,.h = fontHeight };
		renderCopy(texture, &r, 0);
	}
	for(int y = 1; y <= rows; y++) {
		SDL_Texture* texture = gridRowLabels[rows * labelSet + y - 1];
		int fontWidth, fontHeight;
		if(SDL_QueryTexture(texture, NULL, NULL, &fontWidth, &fontHeight) < 0) {
			printf("Error: couldn't get font width and height:\n%s", SDL_GetError());
			exit(1);
		}
		SDL_Rect r = { .x = xOffset + squareWidth / 2 - fontWidth / 2,.y = yOffset + y * squareHeight + squareHeight / 2 - fontHeight / 2,.w = fontWidth,.h = fontHeight };
		renderCopy(texture, &r, 0);
	}
}

void drawHitmap(Hitmap* hitmap, int xOffset, int yOffset) {
	for(int x = 0; x < cols; x++) {
		for(int y = 0; y < rows; y++) {
			SDL_Texture* currentOverlay;

			char field = getHitmapField(hitmap, x, y);
			switch(field) {
				case 0: continue;
				case 1:
					currentOverlay = missedOverlay;
					break;
				case 2:
					currentOverlay = hitOverlay;
					break;
				default:
					fprintf(stderr, "Error: invalid hitmap field.\n");
					exit(1);
			}
			
			SDL_Rect r = { .x = xOffset + x * squareWidth, .y = yOffset + y * squareHeight, .w = squareWidth, .h = squareHeight };
			renderCopy(currentOverlay, &r, 0);
		}
	}
}

void handleAttack(int xOffset, int yOffset, int gridWidth, int gridHeight, char state) {
	int mouseX;
	int mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	if(mouseX > xOffset && mouseX < xOffset + gridWidth && mouseY > yOffset && mouseY < yOffset + gridHeight) {
		int overlayX = mouseX - (mouseX % squareWidth);
		int overlayY = mouseY - (mouseY % squareHeight);
		SDL_Rect r = { .x = overlayX,.y = overlayY,.w = squareWidth,.h = squareHeight };
		renderCopy(mouseOverlay, &r, 0);

		int gridX, gridY;
		convertMouseCoordsToGrid(mouseX, mouseY, xOffset, yOffset, &gridX, &gridY);
		char msg[32];
		sprintf(msg, ATTACK_ONGRID_MSG, gridX + 65, gridY + 1);
		setStatusBar(msg);

		if(state & MOUSE_LEFT_PRESSED) {
			lockMutex(networkState.mutex);
			networkState.clientInfo = 1;
			networkState.x = gridX;
			networkState.y = gridY;
			SDL_CondSignal(networkState.clientSignal);
			SDL_UnlockMutex(networkState.mutex);
		}
	}
	else {
		setStatusBar(ATTACK_MSG);
	}
}

void renderShip(Ship* ship, Uint8 alphaMod, int x, int y, int xOffset, int yOffset) {
	for(int matrixY = 0; matrixY < 5; matrixY++) {
		for(int matrixX = 0; matrixX < 5; matrixX++) {
			if(ship->matrix[matrixY][matrixX] != 0) {
				int partX = (x + matrixX) * squareWidth + xOffset;
				int partY = (y + matrixY) * squareHeight + yOffset;
				SDL_Rect r = { .x = partX,.y = partY,.w = squareWidth,.h = squareHeight };
				SDL_Texture* t;
				switch(ship->matrix[matrixY][matrixX]) {
				case 'F':
					t = shipFront;
					break;
				case 'M':
					t = shipMiddle;
					break;
				case 'B':
					t = shipBack;
					break;
				default:
					printf("Error: %c is not a valid character for a ship part type.\n", ship->matrix[matrixY][matrixX]);
					exit(1);
				}
				setTextureAlphaMod(t, alphaMod);
				renderCopy(t, &r, ((double)ship->rotation) * 90);
				setTextureAlphaMod(t, 255);
			}
		}
	}
}

unsigned char handleShipPlacement(int xOffset, int yOffset, int gridWidth, int gridHeight, char state) {
	if(globalShips[currentShip] == NULL) return 0; // Ignore if ship doesn't exist (probably we're waiting for network thread right now)

	// Obtain mouse coordinates
	int mouseX;
	int mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	// Obtain ship edges coordinates and convert them to global pixel coordinates
	int topEdge;
	int bottomEdge;
	int leftEdge;
	int rightEdge;
	getShipEdges(globalShips[currentShip], &topEdge , &bottomEdge, &leftEdge, &rightEdge);
	topEdge = mouseY + (topEdge - 2) * squareHeight;
	bottomEdge = mouseY + (bottomEdge - 2) * squareHeight;
	leftEdge = mouseX + (leftEdge - 2) * squareWidth;
	rightEdge = mouseX + (rightEdge - 2) * squareWidth;

	if(leftEdge > xOffset && rightEdge < xOffset + gridWidth && topEdge > yOffset && bottomEdge < yOffset + gridHeight) { // If the ship overlay can fit within the grid

		setStatusBar(PLACE_SHIPS_ONGRID_MSG);
		// Calculate ship overlay x and y in grid and set them to ship
		int shipX;
		int shipY;
		convertMouseCoordsToGrid(mouseX, mouseY, xOffset, yOffset, &shipX, &shipY);
		globalShips[currentShip]->x = shipX - 2;
		globalShips[currentShip]->y = shipY - 2;

		if(state & MOUSE_RIGHT_PRESSED) { // Rotate ship
			changeShipRotation(globalShips[currentShip], 5);
			handleShipPlacement(xOffset, yOffset, gridWidth, gridHeight, state & ~MOUSE_RIGHT_PRESSED);
			return 0;
		}

		if(state & MOUSE_LEFT_PRESSED) { // Place ship (if doesn't collide with other placed ships) & switch to next one
			if(!checkCollisionWithPlacedShips(globalShips[currentShip])) {
				for(int i = 0; i < NUMBER_OF_SHIPS; i++) {
					if(ships[i] == 0) {
						ships[i] = globalShips[currentShip];
						globalShips[currentShip] = 0;
						renderShip(ships[i], 255, ships[i]->x, ships[i]->y, xOffset, yOffset);

						// Return 1 if all ships have been placed so the scene can update the client signal if necessary
						unsigned char allPlaced = 1;
						for(int i = 0; i < NUMBER_OF_SHIPS; i++) {
							if(globalShips[i]) allPlaced = 0;
						}
						if(allPlaced) {
							return 1;
						}

						// Otherwise go to next ship
						nextShip();
						return 0;
					}
				}
			}
		}

		if(state & MOUSE_MIDDLE_PRESSED) { // Undo last placement
			int lastShipI;
			unsigned char done = 0;
			for(int i = 0; i <= NUMBER_OF_SHIPS && !done; i++) {
				if(i == NUMBER_OF_SHIPS || ships[i] == 0) {
					lastShipI = i - 1;
					done = 1;
				}
			}
			if(lastShipI != -1) { // There is one or more ships placed
				globalShips[ships[lastShipI]->index] = ships[lastShipI];
				ships[lastShipI] = 0;
			}
			drawShipPlacementOverlay(globalShips[currentShip], xOffset, yOffset, gridWidth, gridHeight);
			previousShip();
			return 0;
		}

		drawShipPlacementOverlay(globalShips[currentShip], xOffset, yOffset, gridWidth, gridHeight);

		if(state & MOUSE_WHEEL_DOWN) {
			nextShip();
			return 0;
		}

		if(state & MOUSE_WHEEL_UP) {
			previousShip();
			return 0;
		}
	}
	else {
		char status[256];
		sprintf(status, PLACE_SHIPS_MSG, opponentNickname);
		setStatusBar(status);
	}
	return 0;
}

void drawShipPlacementOverlay(Ship* ship, int xOffset, int yOffset, int gridWidth, int gridHeight) {
	renderShip(ship, 150, ship->x, ship->y, xOffset, yOffset);
}

void drawPlacedShips(int xOffset, int yOffset) {
	for(int i = 0; i < NUMBER_OF_SHIPS && ships[i] != 0; i++) {
		renderShip(ships[i], 255, ships[i]->x, ships[i]->y, xOffset, yOffset);
	}
}

Ship* checkCollisionWithPlacedShips(Ship* ship) {
	for(int i = 0; i < NUMBER_OF_SHIPS; i++) {
		if(ships[i] != 0) {
			if(checkShipCollision(ship, ships[i])) {
				return ships[i];
			}
		}
	}
	return 0;
}