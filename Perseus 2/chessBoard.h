#pragma once
#include "tables.h"
#include "move.h"
#include <algorithm>

#include <stack>
#define maxPly 64

#define fullDepthMoves 4
#define overReduct 8
#define reductionLimit 3
extern char asciiPieces[13];
int charPieces(char piece);
int getTimeMs();

extern moveInt killerMoves[2][maxPly];
extern moveInt historyMoves[12][64];
extern int ply;
//PV len
extern int pvLen[maxPly];
//pvTable
extern int pvTable[maxPly][64];

struct Position {
	//piece bbs
	U64 bitboards[12] = { 0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL };
	//occ bbs
	U64 occupancies[3] = { 0ULL,0ULL,0ULL };
	
	//data
	int side = -1; //so it needs to be initialized
	// enPassantSquare
	int enPassant = no_square;
	// castling
	int castle = 0;
	//hash key
	int hashKey;

	void print();
	void parseFen(const char* fen);
	void wipe(); //widePopulation() sembrava troppo brutto

	inline bool isSquareAttacked(unsigned int square, int sideToMove);
	inline void operator=(const Position& other);
	void printAttackedSquares(int sideToMove);
	inline void generateMoves(moves* moveList);
	inline void generateCaptures(moves* moveList);
	inline int whiteCaptureValueAt(int square);
	inline int blackCaptureValueAt(int square);
	inline void newKey();
};

void printPosition(Position p);

class Game {
	std::stack<Position> positionStack;
	
public:
	Position pos;
	unsigned int nodes = 0;
	
	
	Game(Position p);
	Game(const char* fen);
	Game();
	void saveState();
	void prevState();
	void print();
	void parseFen(const char* fen);
	void generateMoves(moves* moveList);
	void generateCaptures(moves* moveList);
	void generateLegalMoves(moves* moveList);
	void searchPosition(int depth);
	bool isLegal(const char* moveString);
	moveInt getLegal(const char* moveString);
	inline int eval();
	inline int makeMove(moveInt move, int flags);
	inline int negaMax(int alpha, int beta, int depth,bool pv=false, int nulled=0);
	U64 miniMax(int alpha, int beta, int depth);
	int negaScout(int alpha, int beta, int depth);
	inline int quiescence(int alpha,int beta);
	
};


extern inline unsigned long long perftDriver(int depth, const char *fen);
extern inline unsigned long long perftDriver(int depth, Game* g);
extern inline unsigned long long _perftDriver(int depth, Game* g);
