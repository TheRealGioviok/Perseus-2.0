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
#define kingFileBonus 15
#define centralShield 15
#define sideShield 3
#define bishopColorWeakness 5
#define openFileRook 15


int evaluate(Position* pos) {
	int score = 0;
	U64* bitboards = pos->bitboards;
	U64* occupancies = pos->occupancies;
	int wpawns = popcount(bitboards[0]);
	int bpawns = popcount(bitboards[6]);
	int colorWeakness = 0;
	bool endgame = (popcount(occupancies[both]) <= 12);
	//const int* materialScore = endgame ? materialScoreEnd : materialScoreMid;
	unsigned long square;
	unsigned long wk;
	unsigned long bk;
	bitScanReverse(&wk, bitboards[5]);
	bitScanReverse(&bk, bitboards[11]);

	//int tropismToWhiteKing = 0;
	//int tropismToBlackKing = 0;
	//int whiteScore = 0;
	//int blackScore = 0;

	U64 notWhite = ~(occupancies[white]);
	U64 notBlack = ~(occupancies[black]);
	U64 noneOccupancy = notWhite & notBlack;

	U64 bitboard = bitboards[0];
	//whiteScore += *materialScore * popcount(bitboard);
	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += pawnChainBonus * ((int)popcount(pawnAttacks[0][square] & bitboards[0])) + 3 * ((int)popcount(pawnAttacks[0][square] & (bitboards[7] | bitboards[8]))) - 15 * ((int)popcount(pawnAttacks[0][square] & (bitboards[1] | bitboards[2]))); //pawn chains 
		colorWeakness += 1 - 2 * ((((square % 8) + (square >> 3)) & 1));
		clearBit(bitboard, square);
		score -= 40 * (testBit(occupancies[both], square - 8) > 0);
	}

	//materialScore++;
	bitboard = bitboards[1];
	//whiteScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += -1 * (8 - wpawns);
		score -= 25 * ((pawnAttacks[white][square] & bitboards[6]) > 0);
		score += 3 * popcount((knightAttacks[square]) & noneOccupancy);
		//tropismToBlackKing += nkdist[square][bk];
		clearBit(bitboard, square);
	}

	//materialScore++;
	bitboard = bitboards[2];
	//whiteScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		//score += bishopScore[square]; 
		score += colorWeakness * ((((square % 8) + (square >> 3)) & 1)) * bishopColorWeakness;
		score -= 25 * ((pawnAttacks[white][square] & bitboards[6]) > 0);
		score += 3 * popcount(getBishopAttacks(square, occupancies[both]) & noneOccupancy);
		//tropismToBlackKing += kbdist[square][bk];
		clearBit(bitboard, square);
	}

	//materialScore++;
	bitboard = bitboards[3];
	//whiteScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += ((bitboards[11] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[6] & files[square % 8]) == 0 + (occupancies[black] & files[square % 8]) == 0) * openFileRook;
		//tropismToBlackKing += rkdist[square][bk];
		score += 3 * popcount(getRookAttacks(square, occupancies[both]) & noneOccupancy);
		clearBit(bitboard, square);
	}

	//materialScore++;
	bitboard = bitboards[4];
	//whiteScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score += ((bitboards[11] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[6] & files[square % 8]) == 0 + (occupancies[black] & files[square % 8]) == 0) * openFileRook;
		//tropismToBlackKing += qkdist[square][bk];
		score += popcount(getQueenAttacks(square, occupancies[both]) & noneOccupancy);
		clearBit(bitboard, square);
	}

	//materialScore++;


	if (popcount(pos->bitboards[P]) > popcount(pos->bitboards[p]))score += (8 - (wk >> 3)) << 2;
	/*if (!endgame)
		//score += kingScore[wk];
	else {
		score += kingScoreEnd[wk];
	}
	if (wk > h8) {
		score += (testBit(occupancies[white], wk - 8) > 0) * centralShield;
		score += popcount(occupancies[white]) * sideShield;
	}*/

	//BLACK
	colorWeakness = 0;


	//materialScore++;
	bitboard = bitboards[6];
	//blackScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		bitScanReverse(&square, bitboard);
		score -= pawnChainBonus * ((int)popcount(pawnAttacks[1][square] & bitboards[6])) + 3 * ((int)popcount(pawnAttacks[1][square] & (bitboards[1] | bitboards[2]))) - 15 * ((int)popcount(pawnAttacks[0][square] & (bitboards[7] | bitboards[8]))); //pawn chains 

		colorWeakness += 1 - 2 * ((((square % 8) + (square >> 3)) & 1));
		clearBit(bitboard, square);
		score += 40 * (testBit(occupancies[both], square + 8) > 0);
	}


	//materialScore++;
	bitboard = bitboards[7];
	//blackScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= -1 * (8 - bpawns);
		score += 25 * ((pawnAttacks[black][square] & bitboards[0]) > 0);
		score -= 3 * popcount((knightAttacks[square]) & noneOccupancy);
		//tropismToWhiteKing += nkdist[square][wk];
		clearBit(bitboard, square);
	}


	//materialScore++;
	bitboard = bitboards[8];
	//blackScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		//score -= bishopScore[mirrorScore[square]];
		score -= colorWeakness * ((((square % 8) + (square >> 3)) & 1)) * bishopColorWeakness;
		score += 25 * ((pawnAttacks[black][square] & bitboards[0]) > 0);
		score += 3 * popcount(getBishopAttacks(square, occupancies[both]) & noneOccupancy);

		//tropismToWhiteKing += kbdist[square][wk];
		clearBit(bitboard, square);
	}


	//materialScore++;
	bitboard = bitboards[9];
	//blackScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= ((bitboards[5] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[0] & files[square % 8]) == 0 + (occupancies[black] & files[square % 8]) == 0) * openFileRook;
		//tropismToWhiteKing += rkdist[square][wk];
		score += 3 * popcount(getRookAttacks(square, occupancies[both]) & noneOccupancy);
		clearBit(bitboard, square);
	}


	//materialScore++;
	bitboard = bitboards[10];
	//blackScore += *materialScore * popcount(bitboard);

	while (bitboard) {
		unsigned long square;
		bitScanReverse(&square, bitboard);
		score -= ((bitboards[5] & files[square % 8]) > 0) * kingFileBonus + ((bitboards[0] & files[square % 8]) == 0 + (occupancies[black] & files[square % 8]) == 0) * openFileRook;
		//tropismToWhiteKing += qkdist[square][wk];
		score += popcount(getQueenAttacks(square, occupancies[both]) & noneOccupancy);
		clearBit(bitboard, square);
	}


	//if (!endgame)score -= kingScore[mirrorScore[bk]];
	//else score -= kingScoreEnd[mirrorScore[bk]];
	if (popcount(pos->bitboards[p]) > popcount(pos->bitboards[P]))score -= (bk >> 3) << 2;
	/*
	if (bk < a1) {
		score -= (testBit(occupancies[black], bk + 8) > 0) * centralShield;
		score -= popcount(occupancies[black]) * sideShield;
	}
	*/

	//int whiteKingShield = popcount(bitboards[0] & kingAttacks[wk]) - 3;
	//int blackKingShield = popcount(bitboards[6] & kingAttacks[bk]) - 3;
#define INITIAL_PIECE_MATERIAL 9967
	//int whiteKingSafety = ((whiteKingShield - tropismToWhiteKing) * abs(blackScore)) / INITIAL_PIECE_MATERIAL;
	//int blackKingSafety = ((blackKingShield - tropismToBlackKing) * whiteScore) / INITIAL_PIECE_MATERIAL;

	//score += blackScore + whiteScore + whiteKingSafety - blackKingSafety;
	//std::cout << "BKSAFETY: " << blackKingSafety << " SHIELD IS " << blackKingShield<< "\n";
	//std::cout << "WKSAFETY: " << whiteKingSafety << " SHIELD IS " << whiteKingShield << "\n";
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
	if (!endgame) {


	}
	else {
		score += popcount(kingAttacks[wk] & bitboards[0]) * 5;
		score -= popcount(kingAttacks[bk] & bitboards[6]) * 5;
		score -= 5 * (popcount(bitboards[2]) == 0);
		score += 5 * (popcount(bitboards[8]) == 0);
	}


	return score * (1 - (2 * pos->side));
}