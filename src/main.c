#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "load.h"
#include "game.h"

void printUsageAndQuit(char* programName) {
	fprintf(stderr, "Usage: %s <nickname> [<address> <port>]\nDefault address and port are localhost and 9098.\n", programName);
	exit(1);
}

int main(int argc, char** argv) {
	squareWidth = 60;
	squareHeight = 60;
	cols = 10;
	rows = 10;

	if(argc == 1 || argc == 3 || argc > 4) printUsageAndQuit(argv[0]);
	else {
		if(argc == 2) { // Only nickname provided
			serverAddress = "localhost";
			serverPort = 9098;
			printf("Using default server localhost:9098\nUse %s <nickname> <address> <port> for custom server.\n", argv[0]);
		}
		else {
			serverAddress = argv[2];
			serverPort = strtol(argv[3], NULL, 10);
			if(serverPort < 1 || serverPort > 65535) printUsageAndQuit(argv[0]);
			printf("Using server %s:%ld\n", serverAddress, serverPort);
		}
		nickname = argv[1];
		if(strlen(nickname) > 15) {
			printf("Nickname must be at most 15 characters.\n");
			printUsageAndQuit(argv[0]);
		}
	}

	init();
	gameLoop();
	destroy();

	return 0;
}
