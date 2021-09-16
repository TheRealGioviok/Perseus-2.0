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
void communicate();
void readInput();
int inputWaiting();
U64 calcMoveTime();

extern flag quit;
extern flag movesToGo;
extern U64 moveTime;
extern U64 ucitime;
extern flag inc;
extern U64 startTime;
extern U64 stopTime;
extern flag timeSet;
extern flag stopped;

// (n/16000) of time remained
const U64 timeToUse[] = {
	40ULL, 40ULL, 40ULL, 40ULL,
	40ULL, 40ULL, 40ULL, 40ULL,
	40ULL, 40ULL, 40ULL, 40ULL,
	40ULL, 39ULL, 38ULL, 37ULL,
	36ULL, 35ULL, 34ULL, 33ULL,
	32ULL, 31ULL, 30ULL, 29ULL,
	28ULL, 27ULL, 26ULL, 25ULL,
	24ULL, 23ULL, 22ULL, 21ULL,
	20ULL, 19ULL, 18ULL, 17ULL,
	16ULL, 15ULL, 14ULL, 13ULL,
	12ULL, 11ULL, 10ULL, 9ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL,
	8ULL, 8ULL, 8ULL, 8ULL
};

enum uciMode {
	uciSearch,
	uciInfinite,
	uciTime
};