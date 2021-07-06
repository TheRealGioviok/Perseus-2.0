#pragma once

#include "chessBoard.h"
#include "BBmacros.h"
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
