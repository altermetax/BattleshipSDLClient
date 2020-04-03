#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "load.h"
#include "ship.h"

Ship* makeShip(const char name[20], unsigned char index, int sizeY, int sizeX) {
	Ship* ship = malloc(sizeof(Ship));
	if(ship == NULL) {
		printf("Error: couldn't allocate memory for the ship.\n");
		exit(1);
	}
	strncpy(ship->name, name, 20);
	ship->index = index;
	ship->rotation = 0;
	ship->sizeY = sizeY;
	ship->sizeX = sizeX;
	ship->matrix = allocateAndZeroMatrix(sizeY, sizeX);

	return ship;
}

Ship* copyShip(Ship* src) {
	Ship* dst = makeShip(src->name, src->index, src->sizeX, src->sizeY);
	dst->rotation = src->rotation;
	dst->x = src->x;
	dst->y = src->y;
	copyMatrix(dst->matrix, src->matrix, src->sizeY, src->sizeX);
	return dst;
}

void freeShip(Ship* ship) {
	freeMatrix(ship->matrix, ship->sizeY, ship->sizeX);
}

void nextShip() {
	do {
		currentShip = (currentShip + 1) % NUMBER_OF_SHIPS;
	} while(globalShips[currentShip] == 0);
}

void previousShip() {
	do {
		currentShip = (NUMBER_OF_SHIPS + currentShip - 1) % NUMBER_OF_SHIPS;
	} while(globalShips[currentShip] == 0);
}

void getShipEdges(Ship* ship, int* top, int* bottom, int* left, int* right) {
	*top = 4;
	*bottom = 0;
	*left = 4;
	*right = 0;
	for(int y = 0; y < 5; y++) {
		for(int x = 0; x < 5; x++) {
			if(ship->matrix[y][x] != 0) {
				if(y < *top) *top = y;
				if(y > *bottom) *bottom = y;
				if(x < *left) *left = x;
				if(x > *right) *right = x;
			}
		}
	}
}

void changeShipRotation(Ship* ship, int size) {
	char mirror;
	switch(ship->rotation) {
	case 0:
		ship->rotation = 1;
		mirror = 1;
		break;
	case 1:
		ship->rotation = 2;
		mirror = 0;
		break;
	case 2:
		ship->rotation = 3;
		mirror = 1;
		break;
	case 3:
		ship->rotation = 0;
		mirror = 0;
		break;
	default:
		printf("Error: invalid rotation value for ship: %d.\n", ship->rotation);
		exit(1);
	}

	char** newMatrix = allocateAndZeroMatrix(size, size);
	for(int y = 0; y < size; y++) {
		for(int x = 0; x < size; x++) {
			if(mirror) {
				newMatrix[x][4 - y] = ship->matrix[y][x];
			}
			else {
				newMatrix[x][y] = ship->matrix[y][x];
			}
		}
	}
	copyMatrix(ship->matrix, newMatrix, size, size);
	freeMatrix(newMatrix, size, size);
}

unsigned char checkShipCollision(Ship* ship1, Ship* ship2) {
	int top1, bottom1, left1, right1, top2, bottom2, left2, right2;
	getShipEdges(ship1, &top1, &bottom1, &left1, &right1);
	getShipEdges(ship2, &top2, &bottom2, &left2, &right2);
	top1 += ship1->y;
	bottom1 += ship1->y;
	left1 += ship1->x;
	right1 += ship1->x;
	top2 += ship2->y;
	bottom2 += ship2->y;
	left2 += ship2->x;
	right2 += ship2->x;
	int dx1 = left1 - right2;
	int dx2 = left2 - right1;
	int dy1 = top1 - bottom2;
	int dy2 = top2 - bottom1;
	if(dx1 > 0 || dx2 > 0 || dy1 > 0 || dy2 > 0) return 0;
	return 1;
}

void dynamicStrcat(char** str, char* str2) {
	if(*str != NULL && str2 == NULL) { // reset *str
		free(*str);
		*str = NULL;
		return;
	}
	else if(*str == NULL) { // make new *str
		*str = malloc(strlen(str2) + 1 * sizeof(char));
		strcpy(*str, str2);
	} else { // append str2 to *str
		char* tmp = malloc((strlen(*str) + 1) * sizeof(char));
		strcpy(tmp, *str);
		*str = malloc((strlen(*str) + strlen(str2) + 1) * sizeof(char));
		strcpy(*str, tmp);
		strcpy(*str + strlen(*str), str2);
		free(tmp);
	}
}

char* stringifyShip(Ship* ship) {
	char* string = malloc(128 * sizeof(char));
	string[0] = 0;
	char tmp[128];
	dynamicStrcat(&string, "ship_begin\r\n");
	dynamicStrcat(&string, "name ");
	dynamicStrcat(&string, ship->name);
	dynamicStrcat(&string, "\r\ncoords ");
	sprintf(tmp, "%d %d", ship->x, ship->y);
	dynamicStrcat(&string, tmp);
	dynamicStrcat(&string, "\r\nsize ");
	sprintf(tmp, "%d %d", ship->sizeX, ship->sizeY);
	dynamicStrcat(&string, tmp);
	dynamicStrcat(&string, "\r\nmatrix_begin\r\n");
	for(int y = 0; y < ship->sizeY; y++) {
		for(int x = 0; x < ship->sizeX; x++) {
			if(ship->matrix[y][x] == 0) {
				tmp[0] = '*';
				tmp[1] = 0;
			} else {
				sprintf(tmp, "%c", ship->matrix[y][x]);
			}
			dynamicStrcat(&string, tmp);
		}
		dynamicStrcat(&string, "\r\n");
	}
	dynamicStrcat(&string, "matrix_end\r\nship_end\r\n");
	return string;
}

char* stringifyShips(Ship** ships, int amount) {
	for(int i = 0; i < amount; i++) if(ships[i] == NULL) return NULL;
	char* string = malloc(1024 * sizeof(char));
	string[0] = 0;
	dynamicStrcat(&string, "ships_begin\r\n");
	for(int i = 0; i < amount; i++) {
		char* stringified = stringifyShip(ships[i]);
		dynamicStrcat(&string, stringified);
		free(stringified);
	}
	dynamicStrcat(&string, "ships_end\r\n");
	return string;
}