#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "load.h"
#include "game.h"

void printUsageAndQuit(char* programName) {
	fprintf(stderr, "Usage: %s <address> <port>\n", programName);
	exit(1);
}

int main(int argc, char** argv) {
	squareWidth = 60;
	squareHeight = 60;
	cols = 10;
	rows = 10;

	if(argc == 1) {
	    serverAddress = "localhost";
	    serverPort = 9098;
	    printf("Using default server localhost:9098\nUse %s <address> <port> for custom server.\n", argv[0]);
	}
	else {
        if(argc != 3) printUsageAndQuit(argv[0]);
        serverAddress = argv[1];
        serverPort = strtol(argv[2], NULL, 10);
        if(serverPort <= 0) printUsageAndQuit(argv[0]);
        printf("Using server %s:%ld\n", serverAddress, serverPort);
    }

	init();
	gameLoop();
	destroy();

	return 0;
}
