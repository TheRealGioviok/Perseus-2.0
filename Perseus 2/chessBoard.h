#pragma once
#include "tables.h"
#include "move.h"
#include <algorithm>

#include <stack>

extern char asciiPieces[13];
int charPieces(char piece);
int getTimeMs();



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
	void print();
	void parseFen(const char* fen);
	void wipe(); //widePopulation() sembrava troppo brutto

	inline int isSquareAttacked(unsigned int square, int sideToMove);
	inline void operator=(const Position& other);
	void printAttackedSquares(int sideToMove);
	void generateMoves(moves* moveList);
	int whiteCaptureValueAt(int square);
	int blackCaptureValueAt(int square);
	
};


void printPosition(Position p);

class Game {
	std::stack<Position> positionStack;
	
public:
	Position pos;
	unsigned int nodes = 0;
	int ply=0;
	int bestMove = 0;
	Game(Position p);
	Game(const char* fen);
	Game();
	void saveState();
	void prevState();
	void print();
	void parseFen(const char* fen);
	void generateMoves(moves* moveList);
	void generateLegalMoves(moves* moveList);
	void searchPosition(int depth);
	bool isLegal(const char* moveString);
	moveInt getLegal(const char* moveString);
	inline int eval();
	int makeMove(moveInt move, int flags);
	inline int negaMax(int alpha, int beta, int depth);
	U64 miniMax(int alpha, int beta, int depth);
	int negaScout(int alpha, int beta, int depth);
	int quiescence(int alpha,int beta);
	
};


extern inline unsigned long long perftDriver(int depth, const char *fen);
extern inline unsigned long long perftDriver(int depth, Game* g);
extern inline unsigned long long _perftDriver(int depth, Game* g);
