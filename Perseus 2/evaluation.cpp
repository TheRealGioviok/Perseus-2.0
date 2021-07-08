#include "evaluation.h"
inline int basicEvaluate(Position pos) {
	int score = 0;

	for (int i = 0; i < 12; i++) {
		score += (int)popcount(pos.bitboards[i]) * materialScore[i];
	}
	return score;
}

int evaluate(Position pos) {
	int score = 0;
	
	for (int piece = 0; piece < 6; piece++) {
		U64 bitboard = pos.bitboards[piece];
		score += materialScore[piece] * (int)popcount(bitboard);
		while (bitboard) {
			unsigned long square;
			bitScanReverse(&square, bitboard);
			//eval
			
			switch (piece) {
			case 0:
				score += pawnScore[square] ; //pawn chains + 5 *((int) popcount(pawnAttacks[0][square] & pos.bitboards[0]))
				break;
			case 1:
				score += knightScore[square];
				break;
			case 2:
				score += bishopScore[square];
				break;
			case 3:
				score += rookScore[square];
				break;
			case 4:
				score += queenScore[square];
				break;
			case 5:
				score += kingScore[square];
				break;
			}
			clearBit(bitboard, square);
		}
	}
	for (int piece = 6; piece < 12; piece++) {
		U64 bitboard = pos.bitboards[piece];
		score += materialScore[piece] * (int)popcount(bitboard);
		while (bitboard) {
			unsigned long square;
			bitScanReverse(&square, bitboard);
			//eval
			switch (piece) {
				case 6:
					score -= pawnScore[mirrorScore[square]] ; //pawn chains + 5*((int)popcount(pawnAttacks[1][square]&pos.bitboards[6]))
					break;
				case 7:
					score -= knightScore[mirrorScore[square]];
					break;
				case 8:
					score -= bishopScore[mirrorScore[square]];
					break;
				case 9:
					score -= rookScore[mirrorScore[square]];
					break;
				case 10:
					score -= queenScore[mirrorScore[square]];
					break;
				case 11:
					score += kingScore[mirrorScore[square]];
					break;
			}
			clearBit(bitboard, square);
		}
	}

	//doubled pawns:
	/*
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

	score -= (int)(popcount(pos.bitboards[0] & FILE_A) * popcount(pos.bitboards[0] & IsolatedA)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_B) * popcount(pos.bitboards[0] & IsolatedB)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_C) * popcount(pos.bitboards[0] & IsolatedC)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_D) * popcount(pos.bitboards[0] & IsolatedD)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_E) * popcount(pos.bitboards[0] & IsolatedE)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_F) * popcount(pos.bitboards[0] & IsolatedF)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_G) * popcount(pos.bitboards[0] & IsolatedG)) * 5;
	score -= (int)(popcount(pos.bitboards[0] & FILE_H) * popcount(pos.bitboards[0] & IsolatedH)) * 5;

	score += (int)(popcount(pos.bitboards[6] & FILE_A) * popcount(pos.bitboards[6] & IsolatedA)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_B) * popcount(pos.bitboards[6] & IsolatedB)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_C) * popcount(pos.bitboards[6] & IsolatedC)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_D) * popcount(pos.bitboards[6] & IsolatedD)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_E) * popcount(pos.bitboards[6] & IsolatedE)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_F) * popcount(pos.bitboards[6] & IsolatedF)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_G) * popcount(pos.bitboards[6] & IsolatedG)) * 5;
	score += (int)(popcount(pos.bitboards[6] & FILE_H) * popcount(pos.bitboards[6] & IsolatedH)) * 5;
	*/
	return score * (1-(2*pos.side));
}