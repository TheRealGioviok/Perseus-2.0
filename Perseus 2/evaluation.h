#pragma once
#include "chessBoard.h"
const int materialScoreMid[12] = {
	198,817,836,1270,2521,10000,-198,-817,-836,-1270,-2521,-10000
};
const int materialScoreEnd[12] = {
    258,846,857,1278,2558,10000,-258,-846,-857,-1278,-2558,-10000
};

// pawn positional score
const int pawnScore[64] =
{
    180,  180,  180,  180,  180,  180,  180,  180,
    60,  60,  60,  80,  80,  60,  60,  60,
    40,  40,  40,  60,  60,  60,  40,  40,
    20,  20 , 20,  40,  40,  20,  20,  20,
     10,   10,  20,  40,  40,   20,   10,   10,
     0,   0,   0,   10,   5,   0,   0,   0,
     0,   0,   -2, -30, -30,   -2,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knightScore[64] =
{
    -10,   0,   0,   0,   0,   0,   0,  -10,
    -10,   0,   0,  20,  20,   0,   0,  -10,
    -10,  10,  40,  40,  40,  40,  10,  -10,
    -10,  20,  40,  60,  60,  40,  20,  -10,
    -10,  20,  40,  60,  60,  40,  20,  -10,
    -10,  10,  40,  20,  20,  40,  10,  -10,
    -10,   0,   0,  10,  10,   0,   0,  -10,
    -10, -30,   0,   0,   0,   0, -30,  -10
};

// bishop positional score
const int bishopScore[64] =
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0,
     0,   0,  20,  40,  40,  20,   0,   0,
     0,  30,  20,  40,  40,  20,  30,   0,
     0,  20,   0,   0,   0,   0,  20,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -30,   0,   0, -30,   0,   0

};

// rook positional score
const int rookScore[64] =
{
    100,  100,  100,  100,  100,  100,  100,  100,
    100,  100,  100,  100,  100,  100,  100,  100,
     0,   0,  20,  40,  40,  20,   0,   0,
     0,   0,  20,  40,  40,  20,   0,   0,
     0,   0,  20,  40,  40,  20,   0,   0,
     0,   0,  20,  40,  40,  20,   0,   0,
     0,   0,  20,  40,  40,  20,   0,   0,
     0,  10,   0,  40,  40,   0,   10,   0

};

const int queenScore[64] = {
 -40,-20,-20,-10,-10,-20,-20,-40,
 -20,  0,  0,  0,  0,  0,  0,-20,
 -20,  0, 10, 10, 10, 10,  0,-20,
 -20,  0, 10, 10, 10, 10,  0,-20,
 -20,  0, 10, 10, 10, 10,  0,-20,
 -20,  0, 10, 10, 10, 10,  0,-20,
 -20,  0,  0,  0,  0,  0,  0,-20,
 -40,-20,-20,-10,-10,-20,-20,-40,
};

// king positional score
const int kingScore[64] =
{
      0,    0,   0,   0,  0,    0,    0,   0,
     10,   10,  10,  10,  10,  10,   10,  10,
      0,   10,  10,  20,  20,  10,   10,   0,
      0,   10,  20,  40,  40,  20,   10,   0,
      0,   10,  20,  40,  40,  20,   10,   0,
      0,    0,  20,  40,  40,  20,    0,   0,
      0,   10,  10, -10, -10,   0,   20,   0,
      0,    5,  10,   0, -30,   0,   20,   0
};

// mirror positional score tables for opposite side
const int mirrorScore[128] =
{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

extern inline int basicEvaluate(Position pos);
extern int evaluate(Position pos);



