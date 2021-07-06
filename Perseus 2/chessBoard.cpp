#pragma once
#include "chessBoard.h"
#include "evaluation.h"
#include <windows.h>


int getTimeMs() {
	return GetTickCount();
}

inline unsigned long long perftDriver(int depth,const char *fen=startPosition) {
	U64 nodes = 0;
	int start = getTimeMs();
	Game game(fen);
	game.print();
	nodes = _perftDriver(depth, &game);
	int end = getTimeMs();
	std::cout << nodes << " nodes found at depth " << depth << " in " << end - start << " ms.\n";
	return nodes;
}

inline unsigned long long perftDriver(int depth, Game *game) {
	U64 nodes = 0;
	int start = getTimeMs();
	game->print();
	nodes = _perftDriver(depth, game);
	int end = getTimeMs();
	std::cout << nodes << " nodes found at depth " << depth << " in " << end - start << " ms.\n";
	return nodes;
}

inline int Game::eval() {
	return evaluate(pos);
}

void Game::searchPosition(int depth) {
	bestMove = 0;
	int score = negaMax(-50000, 50000, depth);
	if(bestMove)std::cout << "bmove now : " << bestMove << " -> " << getMoveString(bestMove) << "\n";
}

inline int Game::negaMax(int alpha, int beta, int depth) {
	if (depth == 0)return eval();

	nodes++;

	moves moveList[1];
	generateMoves(moveList);
	moveInt bestSoFar = 0;
	int oldAlpha = alpha;
	Position save = pos;
	//OTTIMIZZAZIONE : PREV POS IS THE SAME THROUGHT THE WHOLE FOR CYCLE, MAYBE NOT SAVING IN MAKEMOVE?
	for (int i = 0; i < moveList->count; i++) {
		pos = save;
		ply++;
		if (makeMove(moveList->moves[i], allMoves) == 0) {
			ply--;
			pos = save;
			continue;
		}
		int score = -negaMax(-beta, -alpha, depth - 1);
		pos = save;
		ply--;

		if (score >= beta) {
			//fail high
			return beta;
		}

		if (score > alpha) {
			// new node
			alpha = score;
			if (ply == 0) {
				bestSoFar = moveList->moves[i];
			}
		}
	}
	if (oldAlpha != alpha) {
		bestMove = bestSoFar;
	}

	return alpha; //fails low
}


U64 _perftDriver(int depth,Game *game)
{
	moves move_list[1];
	int i;
	U64 nodes = 0;

	game->generateMoves(move_list);
	for (i = 0; i < move_list->count; i++) {
		if (game->makeMove(move_list->moves[i], allMoves)) {
			if (depth == 1) nodes++;
			else nodes += _perftDriver(depth - 1, game);
			game->prevState();
		}
	}
	return nodes;
}


char asciiPieces[13] = "PNBRQKpnbrqk";
int charPieces(char piece){
	switch (piece){
		case 'P': return P;
		case 'N': return N;
		case 'B': return B;
		case 'R': return R;
		case 'Q': return Q;
		case 'K': return K;
		case 'p': return p;
		case 'n': return n;
		case 'b': return b;
		case 'r': return r;
		case 'q': return q;
		case 'k': return k;
	}
};
//unicodepieces
//char* unicodePieces[12] = {"","","","","","", "","","", "","",""};
void Position::print() {
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			int square = rank * 8 + file;
			int piece = -1;
			std::cout << ((piece == -1) ? '.' : asciiPieces[piece]);
		}
		std::cout << "\n";
	}
}
void printPosition(Position p) {
	// loop over board ranks 
	for (int rank = 0; rank < 8; rank++) {
		//loop over files
		for (int file = 0; file < 8; file++) {
			int square = rank * 8 + file;
			int piece = -1; //no piece default
			if (!file)std::cout << 8 - rank << " "; //print ranks
			for (int test = 0; test < 12; test++) {
				if (testBit(p.bitboards[test], square))piece = test;
			};
			std::cout << " " << ((piece == -1) ? '.' : asciiPieces[piece]);
		}
		std::cout << "\n";
	}
	std::cout << "\n   a b c d e f g h\n\n";

	//print side
	if (p.side >= 0)std::cout << "   Side to move:     "<<(p.side?" ":"") << ((!p.side) ? "white" : "black") << "\n";
	else std::cout <<	"    Side to move unset! " << "\n";

	//print optional en croissant
	if (p.enPassant != no_square) std::cout << "   En passant square:   " << coords[p.enPassant] << "\n";
	else std::cout <<	"   No en passant square" << "\n";

	//print castling rights
	std::cout << "   Castling rights:   " << ((p.castle & wk) ? "K" : "-") << ((p.castle & wq) ? "Q" : "-") << ((p.castle & bk) ? "k" : "-") << ((p.castle & bq) ? "q" : "-") << "\n\n";

};
void Position::wipe() {
	//reset board position and state

	//reset pos
	memset(bitboards, 0ULL, sizeof(bitboards));

	//reset occupancies
	memset(occupancies, 0ULL, sizeof(occupancies));

	//reset side to move to invalid state
	side = -1;

	//reset optional en croissant square to no_square
	enPassant = no_square;

	//reset castle rights to none
	castle = 0;
}
void Position::parseFen(const char* fen) {
	//std::cout << "Called with:" << std::endl;
	//printf("%s\n", fen);
	wipe();

	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			int square = 8 * rank + file;
			//match ascii pieces within FEN string
			if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'z')) {
				int piece = charPieces(*fen);
				//set piece on corresponding bb
				setBit(bitboards[piece], square);

				//increment pointer
				fen++;
			}
			if (*fen >= '0' && *fen <= '9') {
				//number offset
				int offset = *fen - '0';
				//piece index
				int piece = -1;
				for (int bpiece = P; bpiece <= k; bpiece++) {
					if (testBit(bitboards[bpiece], square))piece = bpiece;
				}
				//on no piece, must decrease file
				if (piece==-1) file--;
				//increase by number offset
				file += offset;
				//increment pointer
				fen++;
			}
			if (*fen == '/')fen++;//increment pointer
		}
	}

	//time to parse side to move
	fen++;
	side = ((*fen == 'w') ? 0 : 1);

	

	//time to parse castling
	fen += 2;
	while (*fen != ' ') {
		switch (*fen) {
		case 'K':
			castle |= wk;
			break;
		case 'Q':
			castle |= wq;
			break;
		case 'k':
			castle |= bk;
			break;
		case 'q':
			castle |= bq;
			break;
		default:
			break;
		}
		fen++;
	}

	//time to parse optional en croissant square
	fen++;

	if (*fen != '-') { //croissant square
		// parse full enPassant file & rank
		int file = *fen - 'a';
		fen++;
		int rank = 8 - (*fen - '0');
		int square = rank * 8 + file;
		fen++;
		enPassant = square;
	}
	else { //no croissant
		enPassant = no_square;
		fen++;
	}

	//WHITE
	for (int piece = P; piece <= K; piece++) {
		//populate occupancy
		occupancies[white] |= bitboards[piece];
	}
	//BLACK
	for (int piece = p; piece <= k; piece++) {
		occupancies[black] |= bitboards[piece];
	}

	occupancies[both] = occupancies[white] | occupancies[black];
	
};
void Position::printAttackedSquares(int sideToMove) {
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			int square = rank * 8 + file;
			if (!file)std::cout << 8 - rank << "  ";
			//check whether current square is attacked or not
			std::cout << ((isSquareAttacked(square, sideToMove)) ? 1 : 0)<<" ";
		}
		std::cout << "\n";
	}
	std::cout << "\n   a b c d e f g h";
}


//RIMEMBRA OTTIMIZZAZIONE OCCUPANCIES BOTH?
inline int Position::isSquareAttacked(unsigned int square, int attackerSide) {
	//attacked by whitePawns
	
		if ((attackerSide == white) && (pawnAttacks[black][square] & bitboards[P]))return 1;

		//attacked by blackPawns
		if ((attackerSide == black) && (pawnAttacks[white][square] & bitboards[p]))return 1;

		//attacked by knights
		if (knightAttacks[square] & ((attackerSide == white) ? bitboards[N] : bitboards[n])) return 1;

		//attacked by bishops
		if (getBishopAttacks(square, occupancies[both]) & ((attackerSide == white) ? bitboards[B] : bitboards[b]))return 1;

		//attacked by rooks
		if (getRookAttacks(square, occupancies[both]) & ((attackerSide == white) ? bitboards[R] : bitboards[r]))return 1;

		//attacked by queens
		if (getQueenAttacks(square, occupancies[both]) & ((attackerSide == white) ? bitboards[Q] : bitboards[q]))return 1;

		//attacked by kings
		if (kingAttacks[square] & ((attackerSide == white) ? bitboards[K] : bitboards[k])) return 1;

	

	return 0;
}
inline void Position::operator=(const Position& other) {

	memcpy(bitboards, other.bitboards, 96);
	memcpy(occupancies, other.occupancies, 24);
	side = other.side;
	enPassant = other.enPassant;
	castle = other.castle;
}

int Position::whiteCaptureValueAt(int square) {
	U64 sqBB = squareBB(square);
	if (bitboards[p] & sqBB) return 10;
	if (bitboards[n] & sqBB) return 30;
	if (bitboards[b] & sqBB) return 30;
	if (bitboards[r] & sqBB) return 50;
	return 90;
}

int Position::blackCaptureValueAt(int square) {
	U64 sqBB = squareBB(square);
	if (bitboards[P] & sqBB) return 10;
	if (bitboards[N] & sqBB) return 30;
	if (bitboards[B] & sqBB) return 30;
	if (bitboards[R] & sqBB) return 50;
	return 90;
}

void Position::generateMoves(moves* moveList) {
	//reset the moveList
	moveList->count = 0;
	//WHITE
	if (side == white) {
		for (int piece = P; piece <= K; piece++) {
			if (piece == P) {
				U64 bitboard = bitboards[P];
				unsigned long sourceSquare, targetSquare;
				while (bitboard) {
					//get next pawn
					bitScanForward(&sourceSquare, bitboard);
					targetSquare = sourceSquare - 8;
					//quiet moves
					if (!testBit(occupancies[both], targetSquare)) {
						//push promotions
						if (targetSquare <= h8) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, Q, 0, 0, 0, 0),90);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, R, 0, 0, 0, 0),50);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, N, 0, 0, 0, 0),40);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, B, 0, 0, 0, 0),30);
						}
						//normal pushes
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 0, 0, 0, 0),0);
							//double pushes
							if (sourceSquare >= a2 && !testBit(occupancies[both], targetSquare - 8)) {
								addMove(moveList, encodeMove(sourceSquare, targetSquare - 8, P, 0, 0, 1, 0, 0),4);
							}
						}
					}

					//pawn attacks
					U64 attacks = pawnAttacks[white][sourceSquare] & occupancies[black];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture promotion
						if (targetSquare <= h8) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, Q, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) + 90);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, R, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) + 50);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, N, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) + 40);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, B, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) + 30);
						}
						//normal pawn capture
						else {
							//std::cout << "It's a me\n";
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare));
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[white][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, P, 0, 1, 0, 1, 0), 111);
						}
					}
					clearBit(bitboard, sourceSquare); //clears analyzed pawn
				}
			}
			else if (piece == K) {
				//is kingside castling availablel?
				if (castle & wk) {
					//free castling target squares
					if (!testBit(occupancies[both], f1) && !testBit(occupancies[both], g1)) {
						//traverse squares and king are not attacked
						if (!isSquareAttacked(e1, black) && !isSquareAttacked(f1, black) && !isSquareAttacked(g1, black)) {
							addMove(moveList, encodeMove(e1, g1, K, 0, 0, 0, 0, 1),8);
						}
					}
				}
				if (castle & wq) {
					//free castling target squares
					if (!testBit(occupancies[both], d1) && !testBit(occupancies[both], c1) && !testBit(occupancies[both], b1)) {
						//traverse squares and king are not attacked
						if (!isSquareAttacked(e1, black) && !isSquareAttacked(d1, black) && !isSquareAttacked(c1, black)) {
							addMove(moveList, encodeMove(e1, c1, K, 0, 0, 0, 0, 1),8);
						}
					}
				}

				unsigned long sourceSquare, targetSquare;
				//only one king, no loop necessary
				bitScanForward(&sourceSquare, bitboards[K]);

				//init attacks
				U64 attacks = kingAttacks[sourceSquare] & (~(occupancies[white]));

				//while not all squares analyzed
				while (attacks) {
					//grab next attack
					bitScanForward(&targetSquare, attacks);
					//captures moves
					if (testBit(occupancies[black], targetSquare)) {
						addMove(moveList, encodeMove(sourceSquare, targetSquare, K, 0, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) - 10);
					}
					//quiet moves
					else {
						addMove(moveList, encodeMove(sourceSquare, targetSquare, K, 0, 1, 0, 0, 0), 0);
					}
					clearBit(attacks, targetSquare);
				}
			}
			else if (piece == N) {
				U64 bitboard = bitboards[N];
				unsigned long sourceSquare, targetSquare;
				//loops over all knights
				while (bitboard) {
					//grab next knight
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = knightAttacks[sourceSquare] & (~(occupancies[white]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[black], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) - 30);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
			else if (piece == B) {
				U64 bitboard = bitboards[B];
				unsigned long sourceSquare, targetSquare;
				//loops over all bishops
				while (bitboard) {
					//grab next bishop
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = getBishopAttacks(sourceSquare, occupancies[both]) & (~(occupancies[white]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[black], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) - 30);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
			else if (piece == R) {
				U64 bitboard = bitboards[R];
				unsigned long sourceSquare, targetSquare;
				//loops over all rooks
				while (bitboard) {
					//grab next rook
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = getRookAttacks(sourceSquare, occupancies[both]) & (~(occupancies[white]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[black], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) - 50);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
			else {
				U64 bitboard = bitboards[Q];
				unsigned long sourceSquare, targetSquare;
				//loops over all queens
				while (bitboard) {
					//grab next queen
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = getQueenAttacks(sourceSquare, occupancies[both]) & (~(occupancies[white]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[black], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 1, 0, 0, 0), 100 + whiteCaptureValueAt(targetSquare) - 90);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}
	//BLACK
	else {
		for (int piece = p; piece <= k; piece++) {
			if (piece == p) {
				U64 bitboard = bitboards[p];
				unsigned long sourceSquare, targetSquare;
				while (bitboard) {
					//get next pawn
					bitScanForward(&sourceSquare, bitboard);
					targetSquare = sourceSquare + 8;
					//quiet moves
					if (!testBit(occupancies[both], targetSquare)) {
						//push promotions
						if (targetSquare >=a1) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 0, 0, 0, 0),90);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 0, 0, 0, 0),50);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 0, 0, 0, 0),40);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 0, 0, 0, 0),30);
						}
						//normal pushes
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 0, 0, 0, 0), 0);
							//double pushes
							if (sourceSquare <= h7  && !testBit(occupancies[both], targetSquare + 8)) {
								addMove(moveList, encodeMove(sourceSquare, targetSquare + 8, p, 0, 0, 1, 0, 0),4);
							}
						}
					}

					//pawn attacks
					U64 attacks = pawnAttacks[black][sourceSquare] & occupancies[white];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture promotion
						if (targetSquare >= a1) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) + 90);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) + 50);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) + 40);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) + 30);
						}
						//normal pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare));
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[black][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, p, 0, 1, 0, 1, 0), 111);
						}
					}
					clearBit(bitboard, sourceSquare); //clears analyzed pawn
				}
			}
			else if (piece == k) {
				//is kingside castling availablel?
				if (castle & bk) {
					//free castling target squares
					if (!testBit(occupancies[both], f8) && !testBit(occupancies[both], g8)) {
						//traverse squares and king are not attacked
						if (!isSquareAttacked(e8, white) && !isSquareAttacked(f8, white) && !isSquareAttacked(g8, white)) {
							addMove(moveList, encodeMove(e8, g8, k, 0, 0, 0, 0, 1),10);
						}
					}
				}
				if (castle & bq) {
					//free castling target squares
					if (!testBit(occupancies[both], d8) && !testBit(occupancies[both], c8) && !testBit(occupancies[both], b8)) {
						//traverse squares and king are not attacked
						if (!isSquareAttacked(e8, white) && !isSquareAttacked(d8, white) && !isSquareAttacked(c8, white)) {
							addMove(moveList, encodeMove(e8, c8, k, 0, 0, 0, 0, 1),10);
						}
					}
				}

				unsigned long sourceSquare, targetSquare;
				//only one king, no loop necessary
				bitScanForward(&sourceSquare, bitboards[k]);

				//init attacks
				U64 attacks = kingAttacks[sourceSquare] & (~(occupancies[black]));

				//while not all squares analyzed
				while (attacks) {
					//grab next attack
					bitScanForward(&targetSquare, attacks);
					//captures moves
					if (testBit(occupancies[white], targetSquare)) {
						addMove(moveList, encodeMove(sourceSquare, targetSquare, k, 0, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) - 10);
					}
					//quiet moves
					else {
						addMove(moveList, encodeMove(sourceSquare, targetSquare, k, 0, 1, 0, 0, 0), 0);
					}
					clearBit(attacks, targetSquare);
				}
			}
			else if (piece == n) {
				U64 bitboard = bitboards[n];
				unsigned long sourceSquare, targetSquare;
				//loops over all knights
				while (bitboard) {
					//grab next knight
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = knightAttacks[sourceSquare] & (~(occupancies[black]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[white], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) - 30);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
			else if (piece == b) {
				U64 bitboard = bitboards[b];
				unsigned long sourceSquare, targetSquare;
				//loops over all bishops
				while (bitboard) {
					//grab next bishop
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = getBishopAttacks(sourceSquare, occupancies[both]) & (~(occupancies[black]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[white], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) - 30);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
			else if (piece == r) {
				U64 bitboard = bitboards[r];
				unsigned long sourceSquare, targetSquare;
				//loops over all rooks
				while (bitboard) {
					//grab next rook
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = getRookAttacks(sourceSquare, occupancies[both]) & (~(occupancies[black]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[white], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) - 50);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
			else {
				U64 bitboard = bitboards[q];
				unsigned long sourceSquare, targetSquare;
				//loops over all queens
				while (bitboard) {
					//grab next queen
					bitScanForward(&sourceSquare, bitboard);
					//init attacks from current square
					U64 attacks = getQueenAttacks(sourceSquare, occupancies[both]) & (~(occupancies[black]));

					//loop over target squares
					while (attacks) {
						//grab next attack
						bitScanForward(&targetSquare, attacks);
						//capture moves
						if (testBit(occupancies[white], targetSquare)) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 1, 0, 0, 0), 100 + blackCaptureValueAt(targetSquare) - 90);
						}
						//quiet moves
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 0, 0, 0, 0), 0);
						}
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}
	std::sort(std::begin(moveList->moves),std::end(moveList->moves),std::greater<int>());
}
void Game::generateMoves(moves* moveList) {
	pos.generateMoves(moveList);
}
void Game::generateLegalMoves(moves* moveList) {
	pos.generateMoves(moveList);
}


int Game::makeMove(moveInt move, int flags) {
	//std::cout << "call\n";
	//allMove makeMove
	if (!move) return 0;
	if (flags == allMoves) {

		Position save = pos;
		
		int source = getMoveSource(move);
		int target = getMoveTarget(move);
		int piece = getMovePiece(move);
		int promotion = getPromotion(move);
		int capture = getCaptureFlag(move);
		int _double = getDoubleFlag(move);
		int enPassant = getEnPassantFlag(move);
		int castle = getCastleFlag(move);

		//move the piece
		
		setBit(pos.bitboards[piece], target);
		clearBit(pos.bitboards[piece], source);

		//handle capture
		if (capture) {
			if (pos.side == white) {	
				if (testBit(pos.bitboards[6], target)) {
					//if piece, remove
					clearBit(pos.bitboards[6], target);
					goto NEXT;
				}
				else if (testBit(pos.bitboards[7], target)) {
					//if piece, remove
					clearBit(pos.bitboards[7], target);
					goto NEXT;
				}
				else if (testBit(pos.bitboards[8], target)) {
					//if piece, remove
					clearBit(pos.bitboards[8], target);
					goto NEXT;
				}
				else if (testBit(pos.bitboards[9], target)) {
					//if piece, remove
					clearBit(pos.bitboards[9], target);
					goto NEXT;
				}
					//if piece, remove
					clearBit(pos.bitboards[10], target);		
			}
			else {
				if (testBit(pos.bitboards[0], target)) {
					//if piece, remove
					clearBit(pos.bitboards[0], target);
					goto NEXT;
				}
				else if (testBit(pos.bitboards[1], target)) {
					//if piece, remove
					clearBit(pos.bitboards[1], target);
					goto NEXT;
				}
				else if (testBit(pos.bitboards[2], target)) {
					//if piece, remove
					clearBit(pos.bitboards[2], target);
					goto NEXT;
				}
				else if (testBit(pos.bitboards[3], target)) {
					//if piece, remove
					clearBit(pos.bitboards[3], target);
					goto NEXT;
				}
				//if piece, remove
				clearBit(pos.bitboards[4], target);
			}
		}
		
		//handle promotion
		NEXT: if (promotion) {
			//erase pawn from target square
			
			clearBit(pos.bitboards[(pos.side?6:0)], target);
			//set promoted piece
			setBit(pos.bitboards[promotion], target);
		}

		//handling croissants
		if (enPassant) {
			(pos.side) ? clearBit(pos.bitboards[P], target - 8) : clearBit(pos.bitboards[p], target + 8);
		}

		

		//handle double pushes (updating croissants)
		if (_double) {
			(pos.side) ? (pos.enPassant = target - 8) : (pos.enPassant = target + 8);
		}
		else {
			//reset croissants
			pos.enPassant = no_square;
		}

		//handling castle
		if (castle) {
			switch (target) {
			case c1:
				clearBit(pos.bitboards[R], a1);
				setBit(pos.bitboards[R], d1);
				break;

			case g1:
				clearBit(pos.bitboards[R], h1);
				setBit(pos.bitboards[R], f1);
				break;

			case c8:
				clearBit(pos.bitboards[r], a8);
				setBit(pos.bitboards[r], d8);
				break;

			case g8:
				clearBit(pos.bitboards[r], h8);
				setBit(pos.bitboards[r], f8);
				break;
			}
		}

		pos.castle &= castlingRights[source];
		pos.castle &= castlingRights[target];

		

		pos.occupancies[0] = pos.bitboards[0] | pos.bitboards[1] | pos.bitboards[2] | pos.bitboards[3] | pos.bitboards[4] | pos.bitboards[5];
		pos.occupancies[1] = pos.bitboards[6] | pos.bitboards[7] | pos.bitboards[8] | pos.bitboards[9] | pos.bitboards[10] | pos.bitboards[11];
		pos.occupancies[2]  = pos.occupancies[1] | pos.occupancies[0];

		// italy moment
		pos.side ^= 1;
		
		// make sure that king has not been exposed into a check
		unsigned long king = 0ULL;
		(pos.side) ? bitScanReverse(&king, pos.bitboards[K]) : bitScanForward(&king, pos.bitboards[k]);
		

		if (pos.isSquareAttacked(king,pos.side))
		{
			// take move back
			pos = save;
			
			// return illegal move
			return 0;
		}


		//pos.side ^= 1;
		//nice move
		
		//positionStack.push(pos);
		//saveState();
		return 1;

	}
	//captures
	else {
		if (getCaptureFlag(move)) 
			return makeMove(move, allMoves);
		else return 0;
	}

	
}


Game::Game() {
	nodes = 0;
	ply = 0;
	bestMove = 0;
	Position ps;
	ps.parseFen(startPosition);
	pos = ps;
	positionStack.emplace(ps);
};

Game::Game(Position ps) {
	nodes = 0;
	ply = 0;
	bestMove = 0;
	pos = ps;
	positionStack.emplace(ps);
}

Game::Game(const char* fen) {
	nodes = 0;
	ply = 0;
	bestMove = 0;
	//std::cout << "oarsing" << "\n";
	pos.parseFen(fen);
	positionStack.emplace(pos);
};

void Game::saveState() {
	positionStack.emplace(pos);
}

void Game::prevState() {
	positionStack.pop();
	pos = positionStack.top();
}

void Game::print() {
	printPosition(pos);
}

void Game::parseFen(const char* fen) {
	pos.parseFen(fen);
}
//parse user/gui input move (ex "d7d8Q")
bool Game::isLegal(const char* moveString) {
	//create moveList instance
	moves moveList[1];
	//generate moves
	generateMoves(moveList);
	for (int i = 0; i < moveList->count; i++) {
		if (!strcmp(getMoveString(moveList->moves[i]).c_str(), moveString))return true;
	}
	return false;
}

moveInt Game::getLegal(const char* moveString) {
	//create moveList instance
	moves moveList[1];
	//generate moves
	generateMoves(moveList);
	for (int i = 0; i < moveList->count; i++) {
		if (!strcmp(getMoveString(moveList->moves[i]).c_str(), moveString))return moveList->moves[i];
	}
	return false;
}