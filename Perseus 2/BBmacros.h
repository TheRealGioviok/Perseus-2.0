#pragma once

#include <string>
typedef unsigned long long U64;
void printBitBoard(U64 bitboard);


//BIT OPERATION MACROS
#define bitPerformance 1

#define testBit(bitboard,square) ((bitboard) & (1ULL<<(square))) //Returns 1 if square-th bit of bitboard is 1, else 0
#define setBit(bitboard,square) ((bitboard)|=(1ULL<<(square))) //sets bitboard square-th bit to 1
#define clearBit(bitboard,square) ((bitboard) &= ~(1ULL << (square))) //clears bitboard square-th bit
#define squareBB(square) ((1ULL<<square)) //gets bitboard equivalent to 0ULL -> setBit(0ULL,square)
#define popcount __popcnt64
#define bitScanForward _BitScanForward64 //ls1b
#define bitScanReverse _BitScanReverse64 //ms1b

//PRESET POSITIONS FOR TEST AND INITIALIZE
#define emptyBoard "8/8/8/8/8/8/8/8 w - - "
#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq e8 0 1 "
#define killerPosition "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "


//enum board squares
enum  {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, no_square
};

//enum side to move
enum {white,black,both};
//enum castle rights
enum { wk = 1, wq = 2, bk = 4, bq = 8};
//enum pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k};
enum {allMoves, onlyCaptures};
// FOR FUTURE ENUMS
extern std::string coords[64];
