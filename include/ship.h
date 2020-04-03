#pragma once

typedef struct {
	char name[20];
	unsigned char index;
	char rotation;
	int x;
	int y;
	int sizeY;
	int sizeX;
	char** matrix;
} Ship;

Ship* makeShip(const char name[20], unsigned char index, int sizeY, int sizeX);
Ship* copyShip(Ship* src);
void freeShip(Ship* ship);
void nextShip();
void previousShip();
void getShipEdges(Ship* ship, int* top, int* bottom, int* left, int* right);
void changeShipRotation(Ship* matrix, int size);
unsigned char checkShipCollision(Ship* ship1, Ship* ship2);
void dynamicStrcat(char** str, char* str2);
char* stringifyShip(Ship* ship);
char* stringifyShips(Ship** ships, int amount);