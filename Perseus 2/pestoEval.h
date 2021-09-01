#include "chessBoard.h"

#define PAWN   0
#define KNIGHT 1
#define BISHOP 2
#define ROOK   3
#define QUEEN  4
#define KING   5

/* board representation */
#define WHITE  0
#define BLACK  1

#define WHITE_PAWN      (2*PAWN   + WHITE)
#define BLACK_PAWN      (2*PAWN   + BLACK)
#define WHITE_KNIGHT    (2*KNIGHT + WHITE)
#define BLACK_KNIGHT    (2*KNIGHT + BLACK)
#define WHITE_BISHOP    (2*BISHOP + WHITE)
#define BLACK_BISHOP    (2*BISHOP + BLACK)
#define WHITE_ROOK      (2*ROOK   + WHITE)
#define BLACK_ROOK      (2*ROOK   + BLACK)
#define WHITE_QUEEN     (2*QUEEN  + WHITE)
#define BLACK_QUEEN     (2*QUEEN  + BLACK)
#define WHITE_KING      (2*KING   + WHITE)
#define BLACK_KING      (2*KING   + BLACK)
#define EMPTY           (BLACK_KING  +  1)


#define PCOLOR(p) ((p)&1)


#define FLIP(sq) ((sq)^56)
#define OTHER(side) ((side)^ 1)

const int mg_value[6] = { 82, 337, 365, 477, 1025,  0 };
const int eg_value[6] = { 94, 281, 297, 512,  936,  0 };
const double drawVals[6] = { 0.3, 0.5, 0.51, 1, 3, 0 };
const double endKingTable[64] = {
	85.75 , 58.5 , 46.25 , 43.0 , 43.0 , 46.25 , 58.5 , 85.75 ,
58.5 , 31.25 , 19.0 , 15.75 , 15.75 , 19.0 , 31.25 , 58.5 ,
46.25 , 19.0 , 6.75 , 3.5 , 3.5 , 6.75 , 19.0 , 46.25 ,
43.0 , 15.75 , 3.5 , 0.25 , 0.25 , 3.5 , 15.75 , 43.0 ,
43.0 , 15.75 , 3.5 , 0.25 , 0.25 , 3.5 , 15.75 , 43.0 ,
46.25 , 19.0 , 6.75 , 3.5 , 3.5 , 6.75 , 19.0 , 46.25 ,
58.5 , 31.25 , 19.0 , 15.75 , 15.75 , 19.0 , 31.25 , 58.5 ,
85.75 , 58.5 , 46.25 , 43.0 , 43.0 , 46.25 , 58.5 , 85.75 ,
};

extern int manhattanDistance[64][64];

/* piece/sq tables */
/* values from Rofchade: http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19 */

extern int mg_pawn_table[64];

extern int eg_pawn_table[64];

extern int mg_knight_table[64];

extern int eg_knight_table[64];

extern int mg_bishop_table[64];

extern int eg_bishop_table[64];

extern int mg_rook_table[64];

extern int eg_rook_table[64];

extern int mg_queen_table[64];

extern int eg_queen_table[64];

extern int mg_king_table[64];

extern int eg_king_table[64];

extern int* mg_pesto_table[6];

extern int* eg_pesto_table[6];

extern int gamephaseInc[12];
extern int mg_table[12][64];
extern int eg_table[12][64];

void init_tables();

int pestoEval(Position* pos);