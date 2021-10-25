#pragma once
#include "BBmacros.h"
#include "move.h"
#include "chessBoard.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "move.h"
//pawn attack tables
#define RANK_1 0xff
#define RANK_2 0xff00
#define RANK_3 0xff0000
#define RANK_4 0xff000000
#define RANK_5 0xff00000000
#define RANK_6 0xff0000000000
#define RANK_7 0xff000000000000
#define RANK_8 0xff00000000000000

#define FILE_A 0x101010101010101
#define FILE_B 0x202020202020202
#define FILE_C 0x404040404040404
#define FILE_D 0x808080808080808
#define FILE_E 0x1010101010101010
#define FILE_F 0x2020202020202020
#define FILE_G 0x4040404040404040
#define FILE_H 0x8080808080808080

#define NOTRANK_1 0xffffffffffffff00
#define NOTRANK_2 0xffffffffffff00ff
#define NOTRANK_3 0xffffffffff00ffff
#define NOTRANK_4 0xffffffff00ffffff
#define NOTRANK_5 0xffffff00ffffffff
#define NOTRANK_6 0xffff00ffffffffff
#define NOTRANK_7 0xff00ffffffffffff
#define NOTRANK_8 0x00ffffffffffffff

#define NOTFILE_A 0xfefefefefefefefe
#define NOTFILE_B 0xfdfdfdfdfdfdfdfd
#define NOTFILE_C 0xfbfbfbfbfbfbfbfb
#define NOTFILE_D 0xf7f7f7f7f7f7f7f7
#define NOTFILE_E 0xefefefefefefefef
#define NOTFILE_F 0xdfdfdfdfdfdfdfdf
#define NOTFILE_G 0xbfbfbfbfbfbfbfbf
#define NOTFILE_H 0x7f7f7f7f7f7f7f7f

const int centerBonusTable10[64] = { 0, 10, 20, 30, 30, 20, 10, 0,
										10, 20, 20, 30, 30, 20, 20, 10,
										0, 10, 30, 30, 30, 30, 10, 0,
										5, 10, 25, 30, 30, 20, 10, 5,
										5, 10, 20, 30, 30, 25, 10, 5,
										0, 10, 30, 30, 30, 30, 10, 0,
										10, 20, 20, 30, 30, 20, 20, 10,
										0, 10, 20, 30, 30, 20, 10, 0 };

//ttable flags

#define hashEXACT 0
#define hashALPHA 1
#define hashBETA 2
//MODIFICA QUESTO
#define hashSize 0x2000000


struct tt {
	U64 key = 0;
	char depth = 0;
	char flags = 0;
	int score = 0;
	int move = 0;
	inline void wipe();
};



const U64 IsolatedA = NOTFILE_B;
const U64 IsolatedB = NOTFILE_A & NOTFILE_C;
const U64 IsolatedC = NOTFILE_B & NOTFILE_D;
const U64 IsolatedD = NOTFILE_C & NOTFILE_E;
const U64 IsolatedE = NOTFILE_D & NOTFILE_F;
const U64 IsolatedF = NOTFILE_E & NOTFILE_G;
const U64 IsolatedG = NOTFILE_F & NOTFILE_H;
const U64 IsolatedH = NOTFILE_G;

const U64 files[8] = { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G };
const U64 ranks[8] = { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7 };
const U64 isolated[8] = { IsolatedA, IsolatedB, IsolatedC, IsolatedD, IsolatedE, IsolatedF, IsolatedG, IsolatedH };
const U64 whiteSquares = 0xaa55aa55aa55aa55;
const U64 blackSquares = 0x55aa55aa55aa55aa;
const U64 squaresOfColour[2] = { whiteSquares, blackSquares };
extern U64 squaresAhead[2][64];

extern U64 pawnAttacks[2][64];
extern U64 knightAttacks[64];
extern U64 kingAttacks[64];
extern const int bishopRelevantBits[64];
extern const int rookRelevantBits[64];
extern U64 rookMagicNumbers[64];
extern U64 bishopMagicNumbers[64];
//bishop attack masks
extern U64 bishopMasks[64];
//rook attack masks
extern U64 rookMasks[64];
//bishop attacks table
extern U64 bishopAttacks[64][512];
//rook attacks table
extern U64 rookAttacks[64][4096];
// random piece keys
extern U64 pieceKeys[12][64];
//random enPassant keys
extern U64 enPassantKeys[65];
//random castling keys
extern U64 castleKeys[16];
//random side key
extern U64 sideKeys;
//tropism
extern int distBonus[64][64];
extern int qkdist[64][64];
extern int rkdist[64][64];
extern int nkdist[64][64];
extern int bkdist[64][64];
extern int kbdist[64][64];
//hashtable
extern tt* hashTable;


extern unsigned int state;
unsigned int getRandomNumber32();
U64 getRandomNumber64();
U64 generateMagicNumber();
U64 findMagicNumber(int square, int relevantBits, int bishop);
void initMagicNumbers();

void initializePawnAttacks();
void initializeLeaperAttacks();
void initAll();
void initBBTables();
void initTropism();

U64 setOccupancy(int index, __int64 bitsInMask, U64 attackMask);
U64 maskPawnAttacks(int square, int sideToMove); //generate pawn attacks for square-side to move
U64 maskBishopAttacks(int square);
U64 maskRookAttacks(int square);
U64 bishopAttacksOnTheFly(int square, U64 block);
U64 rookAttacksOnTheFly(int square, U64 block);

void initSlidersAttacks(int bishop);
void initHashKeys();
extern inline U64 getBishopAttacks(int square, U64 occupancy);
extern inline U64 getRookAttacks(int square, U64 occupancy);
extern inline U64 getQueenAttacks(int square, U64 occupancy);
extern inline void wipeTT();
extern inline void ageTT();

extern inline int readHashEntry(U64 key, int alpha, int beta, int depth);
extern inline void writeHashEntry(U64 key, int score, int depth, moveInt move, int hash_flag);
extern inline tt* getEntry(U64 key);