#pragma once
/*
0000 0000 0000 0000 00xx xxxx  source square		0x3f
0000 0000 0000 xxxx xx00 0000  target square		0xfc0
0000 0000 xxxx 0000 0000 0000  piece				0xf000
0000 xxxx 0000 0000 0000 0000  promoted piece		0xf0000
0001 0000 0000 0000 0000 0000  capture flag			0x100000
0010 0000 0000 0000 0000 0000  double push flag		0x200000
0100 0000 0000 0000 0000 0000  enpassant flag		0x400000
1000 0000 0000 0000 0000 0000  castling flag		0x800000 
*/
#include <string>
typedef int moveInt;
#define encodeMove(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

#define getMoveSource(move)		( move & 0x3f    )
#define getMoveTarget(move)		((move & 0xfc0   ) >>  6 )
#define getMovePiece(move)		((move & 0xf000  ) >> 12 )
#define getPromotion(move)		((move & 0xf0000 ) >> 16 )


#define getCaptureFlag(move)    ((move & 0x100000))
#define getDoubleFlag(move)		((move & 0x200000))
#define getEnPassantFlag(move)  ((move & 0x400000))
#define getCastleFlag(move)		((move & 0x800000))
#define isCapture(move)			((move & 0x100000) >> 20)
#define isDouble(move)			((move & 0x200000) >> 21)
#define isEnPassant(move)		((move & 0x400000) >> 22)
#define isCastle(move)			((move & 0x800000) >> 23)

struct moves {
	//moves
	int moves[256];
	
	//move count
	int count = 0;

};

char promotedPieces(int piece);
std::string getMoveString(moveInt move);
void printMove(moveInt move);
void printMoveList(moves* moveList);
void addMove(moves* moveList, moveInt move, int bonus = 0);
static inline void orderedInsert(moves *moveList, moveInt move);
const int castlingRights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};