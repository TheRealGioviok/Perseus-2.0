#include "evaluation.h"
#include <cmath>
inline int basicEvaluate(Position* pos) {
	int score = 0;

	for (int i = 0; i < 12; i++) {
		score += (int)popcount(pos->bitboards[i]) * materialScoreMid[i];
	}
	return score;
}

#define pawnChainBonus 5
#define kingFileBonus 5
#define centralShield 15
#define sideShield 3
#define bishopColorWeakness 5
#define openFileRook 10


int evaluate(Position* pos) {
	int score = 0;
	U64* bitboards = pos->bitboards;
	U64* occupancies = pos->occupancies;
	int wpawns = popcount(bitboards[0]);
	int bpawns = popcount(bitboards[6]);
	int colorWeakness = 0;
	bool endgame = (popcount(occupancies[both]) <= 12);
	const int* materialScore = endgame?materialScoreEnd:materialScoreMid;
	unsigned long square;

	U64 bitboard = bitboards[0];
	score += *materialScore * popcount(bitboard);
	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += pawnScore[square] + pawnChainBonus * ((int)popcount(pawnAttacks[0][square] & bitboards[0])) + 3 * ((int)popcount(pawnAttacks[0][square] & (bitboards[7] | bitboards[8]))); //pawn chains 
		colorWeakness += 1 - 2 * ((square % 8 + square >> 8) & 1);
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = bitboards[1];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += knightScore[square] - 1 * (8 - wpawns);
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = bitboards[2];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += bishopScore[square];
		score += colorWeakness * ((square % 8 + square >> 8) & 1) * bishopColorWeakness;
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = bitboards[3];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += rookScore[square] + ((bitboards[11] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[6] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}

	materialScore++;
	bitboard = bitboards[4];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += queenScore[square] + ((bitboards[11] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[6] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}

	materialScore++;

	bitScanReverse(&square, bitboards[5]);
	if (popcount(pos->bitboards[P]) > popcount(pos->bitboards[p]))score += (8 - (square >> 3)) << 2;
	if(!endgame)
		score += kingScore[square];
	else {
		score += kingScoreEnd[square];
	}
	if (square >h8) {
		score += (testBit(occupancies[white], square - 8)>0) * centralShield;
		score += popcount(occupancies[white]) * sideShield;
	}

	//BLACK
	colorWeakness = 0;


	materialScore++;
	bitboard = bitboards[6];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score -= pawnScore[mirrorScore[square]] + pawnChainBonus * ((int)popcount(pawnAttacks[1][square] & bitboards[6])) + 3 * ((int)popcount(pawnAttacks[1][square] & (bitboards[1] | bitboards[2]))); //pawn chains 
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = bitboards[7];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= knightScore[mirrorScore[square]] - 1 * (8 - bpawns); 
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = bitboards[8];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= bishopScore[mirrorScore[square]];
		score -= colorWeakness * ((square % 8 + square >> 8) & 1) * bishopColorWeakness;
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = bitboards[9];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= rookScore[mirrorScore[square]] + ((bitboards[5] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[0] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}


	materialScore++;
	bitboard = bitboards[10];
	score += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= queenScore[mirrorScore[square]] + ((bitboards[5] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[0] & files[square % 8]) == 0) * openFileRook;
		clearBit(bitboard, square);
	}

	bitScanReverse(&square, bitboards[11]);
	if(!endgame)score -= kingScore[mirrorScore[square]];
	else score -= kingScoreEnd[mirrorScore[square]];
	if (popcount(pos->bitboards[p]) > popcount(pos->bitboards[P]))score -= (square >> 3) << 2;
	if (square < a1) {
		score -= (testBit(occupancies[black], square + 8)>0) * centralShield;
		score -= popcount(occupancies[black]) * sideShield;
	}
	
	//doubled pawns:
	
	score -= (int)(popcount(bitboards[0] & FILE_A) - 1) * 20;
	score -= (int)(popcount(bitboards[0] & FILE_B) - 1) * 15;
	score -= (int)(popcount(bitboards[0] & FILE_C) - 1) * 15;
	score -= (int)(popcount(bitboards[0] & FILE_D) - 1) * 25;
	score -= (int)(popcount(bitboards[0] & FILE_E) - 1) * 25;
	score -= (int)(popcount(bitboards[0] & FILE_F) - 1) * 15;
	score -= (int)(popcount(bitboards[0] & FILE_G) - 1) * 15;
	score -= (int)(popcount(bitboards[0] & FILE_H) - 1) * 20;

	score += (int)(popcount(bitboards[6] & FILE_A) - 1) * 20;
	score += (int)(popcount(bitboards[6] & FILE_B) - 1) * 15;
	score += (int)(popcount(bitboards[6] & FILE_C) - 1) * 15;
	score += (int)(popcount(bitboards[6] & FILE_D) - 1) * 25;
	score += (int)(popcount(bitboards[6] & FILE_E) - 1) * 25;
	score += (int)(popcount(bitboards[6] & FILE_F) - 1) * 15;
	score += (int)(popcount(bitboards[6] & FILE_G) - 1) * 15;
	score += (int)(popcount(bitboards[6] & FILE_H) - 1) * 20;

	for (int file = 0; file < 8; file++) {
		if (bitboards[0] & files[file]) {
			if (bitboards[6] & ((files[file]) == 0)) {
				score += 10;
				if (occupancies[black] & ((files[file]) == 0)) {
					score += 20;
				}
			}
		}
		else if (bitboards[6] & files[file]) {
			if (bitboards[0] & ((files[file]) == 0)) {
				score -= 10;
				if (occupancies[white] & ((files[file]) == 0)) {
					score -= 20;
				}
			}
		}
	}
	
	score += 10 * (popcount(bitboards[2]) >= 2);
	score -= 10 * (popcount(bitboards[8]) >= 2);

	//endgame
	if (endgame) {
		score += popcount(kingAttacks[wk] & bitboards[0]) * 5;
		score -= popcount(kingAttacks[bk] & bitboards[6]) * 5;
		score -= 5 * (popcount(bitboards[2]) == 0);
		score += 5 * (popcount(bitboards[8]) == 0);
		
	}

	return score * (1-(2*pos->side));
}