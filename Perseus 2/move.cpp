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
	for (int i = 0; i < moveList->count; i++) {
		printMove(moveList->m[i]);
	}
}

void addMove(moves* moveList, moveInt move, int bonus ) {
	moveInt oMove = onlyMove(move);
	for (int i = 0; i < ply; i++) {
		if (pvTable[0][ply] == oMove) {
			move += ((255-i) << 24);
			moveList->m[moveList->count] = move;
			moveList->count++;
			return;
		}
	}

	if (killerMoves[0][ply] == oMove) {
		bonus += 60;
	}
	else if (killerMoves[1][ply] == oMove) {
		bonus += 59;
	}
	
	int s = getMoveSource(move);
	bonus += (int)((3.5f - abs(3.5f - (float)(s & 0x7))));
	bonus += (int)((3.5f - abs(3.5f - (float)(s / 8))));
	bonus /= 4;
	move = (bonus << 24) + move;
	moveList->m[moveList->count] = move;
	moveList->count++;
}

