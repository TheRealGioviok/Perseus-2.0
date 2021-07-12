#pragma once

#include "chessBoard.h"
#include "BBmacros.h"
//#include <stdlib.h>

typedef int flag;
/*


UCI COMMANDS

position:
	startpos
	startpos moves e2e4 e7e5 ...
	fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq e8 0 1
	fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq e8 0 1 moves e2a6 e8g8 ...
*/

moveInt parseNormalMove(char* moveString);

void parseCommand(std::string, Game* game);
void uciLoop();
/*static void communicate();
void readInput();
int inputWaiting();


extern flag quit;
extern flag movesToGo;
extern flag moveTime;
extern flag ucitime;
extern flag inc;
extern flag startTime;
extern flag stopTime;
extern flag timeSet;
extern flag stopped;*/