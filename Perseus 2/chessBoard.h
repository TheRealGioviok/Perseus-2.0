#pragma once
#include "tables.h"
#include "move.h"
#include <algorithm>


#include <stack>
#define maxPly 64

#define fullDepthMoves 4
#define overReduct 8
#define reductionLimit 3

#define infinity 50000
#define mateValue 49000
#define mateScore 48000

extern char asciiPieces[13];
int charPieces(char piece);
U64 getTimeMs();

extern moveInt killerMoves[2][maxPly];
extern int historyMoves[12][64];
extern moveInt counterMoves[64][64];
extern U64 repetitionTable[128];
extern int repetitionIndex;
extern int ply;
//PV len
extern int pvLen[maxPly];
//pvTable
extern int pvTable[maxPly][64];

struct Position {
	//piece bbs
	U64 bitboards[12] = { 0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL };
	//occ bbs
	//U64 occupancies[3] = { 0ULL,0ULL,0ULL };
	
	//data
	signed char side = -1; //so it needs to be initialized
	// enPassantSquare
	char enPassant = no_square;
	// castling
	signed char castle = 0;
	//hash key
	int hashKey;
	int lastMove = 0;

	void print();
	void parseFen(const char* fen);
	void wipe(); //widePopulation() sembrava troppo brutto

	inline bool isSquareAttacked(unsigned int square, int sideToMove);
	//inline void operator=(const Position& other);
	void printAttackedSquares(int sideToMove);
	inline void generateMoves(moves* moveList, bool isCheck = false);
	inline void generateCaptures(moves* moveList);
	inline int whiteCaptureValueAt(int square);
	inline int blackCaptureValueAt(int square);
	
	inline void newKey();
	inline void addMove(moves* moveList, moveInt move, int bonus = 0);
	
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
	void generateMoves(moves* moveList, bool isCheck = false);
	void generateCaptures(moves* moveList);
	void generateLegalMoves(moves* moveList);
	void searchPosition(int depth);
	bool isLegal(const char* moveString);
	bool moveLegal(moveInt move);
	inline bool isRepetition();
	moveInt getLegal(const char* moveString);
	inline int eval();
	inline int makeMove(moveInt move, int flags = allMoves);
	inline int negaMax(int alpha, int beta, int depth,bool pv=false, double nulled = 0);
	inline int negaMax2(int alpha, int beta, int depth, bool cutNode);
	U64 miniMax(int alpha, int beta, int depth);
	int negaScout(int alpha, int beta, int depth);
	moveInt IID(moves* moveList, int depth = 3);
	inline int quiescence(int alpha,int beta);
	
	
};


extern inline unsigned long long perftDriver(int depth, const char *fen);
extern inline unsigned long long perftDriver(int depth, Game* g);
extern inline unsigned long long _perftDriver(int depth, Game* g);
