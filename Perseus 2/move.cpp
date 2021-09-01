#pragma once
#include "move.h"
#include "chessBoard.h"
#include <iostream>
#include <cassert>
#include "BBmacros.h"

char promotedPieces(int piece) {
	switch (piece) {
	case Q:case q:
		return 'q';
	case N:case n:
		return 'n';
	case R:case r:
		return 'r';
	case B:case b:
		return 'b';
	default:
		return ' ';
	}
}

void printMove(moveInt move) {
	std::cout << coords[getMoveSource(move)] << coords[getMoveTarget(move)] << promotedPieces(getPromotion(move)) << "\n";
}

std::string getMoveString(moveInt move) {
	std::string res = coords[getMoveSource(move)] + coords[getMoveTarget(move)];
	if (getPromotion(move))res += promotedPieces(getPromotion(move));
	return res;
}

void printMoveList(moves* moveList) {
	for (int i = 0; i < moveList->count; ++i) {
		printMove(moveList->m[i]);
	}
}


