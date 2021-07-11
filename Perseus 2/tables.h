#pragma once
#include "BBmacros.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
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

const U64 IsolatedA = NOTFILE_B;
const U64 IsolatedB = NOTFILE_A & NOTFILE_C;
const U64 IsolatedC = NOTFILE_B & NOTFILE_D;
const U64 IsolatedD = NOTFILE_C & NOTFILE_E;
const U64 IsolatedE = NOTFILE_D & NOTFILE_F;
const U64 IsolatedF = NOTFILE_E & NOTFILE_G;
const U64 IsolatedG = NOTFILE_F & NOTFILE_H;
const U64 IsolatedH = NOTFILE_G;

const U64 files[8] = { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G };

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


extern unsigned int state;
unsigned int getRandomNumber32();
U64 getRandomNumber64();
U64 generateMagicNumber();
U64 findMagicNumber(int square, int relevantBits, int bishop);
void initMagicNumbers();

void initializePawnAttacks();
void initializeLeaperAttacks();
void initAll();


U64 setOccupancy(int index, __int64 bitsInMask, U64 attackMask);
U64 maskPawnAttacks(int square, int sideToMove); //generate pawn attacks for square-side to move
U64 maskBishopAttacks(int square);
U64 maskRookAttacks(int square);
U64 bishopAttacksOnTheFly(int square, U64 block);
U64 rookAttacksOnTheFly(int square, U64 block);

void initSlidersAttacks(int bishop);
extern inline U64 getBishopAttacks(int square, U64 occupancy);
extern inline U64 getRookAttacks(int square, U64 occupancy);
extern inline U64 getQueenAttacks(int square, U64 occupancy);