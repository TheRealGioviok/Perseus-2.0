#pragma once
#include "move.h"
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
		printMove(moveList->moves[i]);
	}
}

void addMove(moves* moveList, moveInt move, int bonus ) {
	unsigned int score = 0;
	
	score += bonus; //includes MVV - LVA

	score += (int)((3.5f - abs(3.5f - (float)(getMoveSource(move) & 0x7))));
	score += (int)((3.5f - abs(3.5f - (float)(getMoveSource(move) / 8))) );
	

	score /= 4;
	score += abs((getMoveSource(move) / 8) - (getMoveTarget(move) / 8));
	
	//std::cout << getMoveString(move)<<" -> move before the shifted bonus: " << std::hex << move << std::endl;
	//std::cout << "Move " << getMoveString(move) << " has been assigned a bonus of " << score << ".\n";
	move = (score << 24) + move;
	//std::cout << getMoveString(move) << " -> move after the shifted bonus: " << std::hex << move << std::endl;
	moveList->moves[moveList->count] = move;
	moveList->count++;
}

