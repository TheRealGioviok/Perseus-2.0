#include "evaluation.h"
#include <cmath>
inline int basicEvaluate(Position pos) {
	int score = 0;

	for (int i = 0; i < 12; i++) {
		score += (int)popcount(pos.bitboards[i]) * materialScoreMid[i];
	}
	return score;
}

#define pawnChainBonus 5
#define kingFileBonus 5
#define centralShield 15
#define sideShield 3
#define bishopColorWeakness 5
#define openFileRook 10


int evaluate(Position pos) {
	int score = 0;
	int wpawns = popcount(pos.bitboards[0]);
	int bpawns = popcount(pos.bitboards[6]);
	int colorWeakness = 0;
	bool endgame = (popcount(pos.occupancies[both]) <= 12);
	const int* materialScore = endgame?materialScoreEnd:materialScoreMid;
	unsigned long square;

	U64 bitboard = pos.bitboards[0];
	score += *materialScore * popcount(bitboard);
	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += pawnScore[square] + pawnChainBonus * ((int)popcount(pawnAttacks[0][square] & pos.bitboards[0])) + 3 * ((int)popcount(pawnAttacks[0][square] & (pos.bitboards[7] | pos.bitboards[8]))); //pawn chains 
		colorWeakness += 1 - 2 * ((square % 8 + square >> 8) & 1);
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = pos.bitboards[1];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += knightScore[square] - 1 * (8 - wpawns);
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = pos.bitboards[2];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += bishopScore[square];
		score += colorWeakness * ((square % 8 + square >> 8) & 1) * bishopColorWeakness;
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = pos.bitboards[3];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += rookScore[square] + ((pos.bitboards[11] & files[square % 8]) > 0) * kingFileBonus + ((pos.bitboards[6] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = pos.bitboards[4];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += queenScore[square] + ((pos.bitboards[11] & files[square % 8]) > 0) * kingFileBonus + ((pos.bitboards[6] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}

	materialScore++;

	bitScanReverse(&square, pos.bitboards[11]);
	score += kingScore[square];
	if (square >h8) {
		score += (testBit(pos.occupancies[white], square - 8)>0) * centralShield;
		score += popcount(pos.occupancies[white]) * sideShield;
	}

	//BLACK
	colorWeakness = 0;


	materialScore++;
	bitboard = pos.bitboards[6];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score -= pawnScore[mirrorScore[square]] + pawnChainBonus * ((int)popcount(pawnAttacks[1][square] & pos.bitboards[6])) + 3 * ((int)popcount(pawnAttacks[1][square] & (pos.bitboards[1] | pos.bitboards[2]))); //pawn chains 
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = pos.bitboards[7];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= knightScore[mirrorScore[square]] - 1 * (8 - bpawns); 
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = pos.bitboards[8];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= bishopScore[mirrorScore[square]];
		score -= colorWeakness * ((square % 8 + square >> 8) & 1) * bishopColorWeakness;
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = pos.bitboards[9];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= rookScore[mirrorScore[square]] + ((pos.bitboards[5] & files[square % 8]) > 0) * kingFileBonus + ((pos.bitboards[0] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = pos.bitboards[10];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= queenScore[mirrorScore[square]] + ((pos.bitboards[5] & files[square % 8]) > 0) * kingFileBonus + ((pos.bitboards[0] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}

	bitScanReverse(&square, pos.bitboards[11]);
	score -= kingScore[mirrorScore[square]];
	if (square < a1) {
		score -= (testBit(pos.occupancies[black], square + 8)>0) * centralShield;
		score -= popcount(pos.occupancies[black]) * sideShield;
	}
	
	//doubled pawns:
	
	score -= (int)(popcount(pos.bitboards[0] & FILE_A) - 1) * 20;
	score -= (int)(popcount(pos.bitboards[0] & FILE_B) - 1) * 15;
	score -= (int)(popcount(pos.bitboards[0] & FILE_C) - 1) * 15;
	score -= (int)(popcount(pos.bitboards[0] & FILE_D) - 1) * 25;
	score -= (int)(popcount(pos.bitboards[0] & FILE_E) - 1) * 25;
	score -= (int)(popcount(pos.bitboards[0] & FILE_F) - 1) * 15;
	score -= (int)(popcount(pos.bitboards[0] & FILE_G) - 1) * 15;
	score -= (int)(popcount(pos.bitboards[0] & FILE_H) - 1) * 20;

	score += (int)(popcount(pos.bitboards[6] & FILE_A) - 1) * 20;
	score += (int)(popcount(pos.bitboards[6] & FILE_B) - 1) * 15;
	score += (int)(popcount(pos.bitboards[6] & FILE_C) - 1) * 15;
	score += (int)(popcount(pos.bitboards[6] & FILE_D) - 1) * 25;
	score += (int)(popcount(pos.bitboards[6] & FILE_E) - 1) * 25;
	score += (int)(popcount(pos.bitboards[6] & FILE_F) - 1) * 15;
	score += (int)(popcount(pos.bitboards[6] & FILE_G) - 1) * 15;
	score += (int)(popcount(pos.bitboards[6] & FILE_H) - 1) * 20;

	for (int file = 0; file < 8; file++) {
		if (pos.bitboards[0] & files[file]) {
			if (pos.bitboards[6] & ((files[file]) == 0)) {
				score += 10;
				if (pos.occupancies[black] & ((files[file]) == 0)) {
					score += 20;
				}
			}
		}
		else if (pos.bitboards[6] & files[file]) {
			if (pos.bitboards[0] & ((files[file]) == 0)) {
				score -= 10;
				if (pos.occupancies[white] & ((files[file]) == 0)) {
					score -= 20;
				}
			}
		}
	}
	
	score += 10 * (popcount(pos.bitboards[2]) >= 2);
	score -= 10 * (popcount(pos.bitboards[8]) >= 2);

	//endgame
	if (endgame) {
		score += popcount(kingAttacks[wk] & pos.bitboards[0]) * 5;
		score -= popcount(kingAttacks[bk] & pos.bitboards[6]) * 5;
		score -= 5 * (popcount(pos.bitboards[2]) == 0);
		score += 5 * (popcount(pos.bitboards[8]) == 0);
	}

	return score * (1-(2*pos.side));
}