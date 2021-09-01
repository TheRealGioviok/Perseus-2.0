#pragma once
#include "chessBoard.h"
#include "evaluation.h"
#include "pestoEval.h"
#include <windows.h>

moveInt killerMoves[2][maxPly] = { {0} };
int historyMoves[12][64] = { {0	} };
moveInt counterMoves[64][64] = { {0} };
U64 repetitionTable[128] = { 0 };
int repetitionIndex = 0;

/*

++repetitionIndex;
repetitionTable[repetitionIndex] = pos.hashKey;


*/
int ply=0;
//PV len
int pvLen[maxPly] = { 0 };
//pvTable
int pvTable[maxPly][64] = { {0} };

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
#define pesto true
#if pesto
	return pestoEval(&pos);
	//return (int)(1.55*pestoEval(&pos) + 0.45*evaluate(&pos))/2;
	return evaluate(&pos);
#endif
}
#define VALWINDOW 20
void Game::searchPosition(int depth) {

	int score = 0;
	int alpha = -50000;
	int beta = 50000;
	//stopped = false;

	memset(killerMoves, 0, sizeof(killerMoves));
	memset(pvTable, 0, sizeof(pvTable));
	memset(pvLen, 0, sizeof(pvLen));
	memset(historyMoves, 0, sizeof(historyMoves));
	memset(counterMoves, 0, sizeof(counterMoves));

	for (int c = 2; c <= depth; c += 1) {
		//if (stopped == true)break;
		nodes = 0;
		//enable followPv
		//
		std::cout << "Window of search: " << alpha << " " << beta << "\n";
		int timer = getTimeMs();
		
		
		
		score = negaMax(alpha, beta, c);

		if (alpha != -50000 && beta != 50000) {
			for (int i = 4; i < 16; i*=2) {
				if (score <= alpha) alpha -= VALWINDOW * i;
				else if (score >= beta) beta += VALWINDOW * i;
				else { break; }
				std::cout << "(Research) Window of search: " << alpha << " " << beta << "\n";
				score = negaMax(alpha, beta, c);
			}
		}
		
		if (score <= alpha || score >= beta) {
			score = negaMax(-50000, 50000, c);
		}

		int time2 = getTimeMs();
		
		alpha = score - VALWINDOW;
		beta = score + VALWINDOW;

		if (score<-mateScore && score >-mateValue) {
			std::cout << "info score mate " << -(score+mateValue)/2-1 << " depth " << c << " nodes " << nodes << " pv ";
		}
		else if (score<mateValue && score >mateScore) {
			std::cout << "info score mate " << (mateValue-score)/2+1 << " depth " << c << " nodes " << nodes << " pv ";
		}
		else {
			std::cout << "info score cp " << score << " depth " << c << " nodes " << nodes << " pv ";
		}
		
		
		for (int i = 0; i < pvLen[0]; ++i) {
			historyMoves[getMovePiece(pvTable[0][i])][getMoveTarget(pvTable[0][i])] += c + 1;
			std::cout << getMoveString(pvTable[0][i]) << " ";
		}
		std::cout << " speed "<< (nodes / (time2 - timer + 1)) << "kN/S"<<std::endl;
		if (abs(score) > 48000 && c >= 8)break;
		//if (c + 2 > depth && nodes < 1000000)depth += 2;
	}
	std::cout << "bestmove " << getMoveString(pvTable[0][0]) << std::endl;
	wipeTT();

	//std::cout << "bmove now : " << bestMove << " -> " << getMoveString(bestMove) << "\n";

}

bool Game::moveLegal(moveInt move) {
	if (pos.bitboards[getMovePiece(move)] & squareBB(getMoveSource(move))) {
		return true;
	}
	return false;
}

inline bool Game::isRepetition() {
	//loop over rep indicies
	for (int index = 0; index < repetitionIndex; ++index) {
		if (pos.hashKey == repetitionTable[index])return true;
	}
	//if no repetition
	return false;
}

inline int Game::quiescence(int alpha, int beta) {
	
	//if ((nodes & 2047) == 0) communicate();
	++nodes;

	//legal counter
	int eeval = eval();

	if (ply > maxPly - 1) {
		return eeval;
	}

	//fail hard beta
	if (eeval >= beta)return beta;

	//DELTA VALUE
#define BIGDELTA 775 // queen value
//if (isPromotingPawn()) BIG_DELTA += 775;

	if (eeval < alpha - BIGDELTA) {
		return alpha;
	}


	if (eeval > alpha)alpha = eeval;
	moves* moveList = new moves;
	generateCaptures(moveList);
	Position save = pos;
	//OTTIMIZZAZIONE : PREV POS IS THE SAME THROUGHT THE WHOLE FOR CYCLE, MAYBE NOT SAVING IN MAKEMOVE?
	for (int i = 0; i < moveList->count; ++i) {

		++ply;
		if (makeMove(moveList->m[i]) == 0) {
			--ply;
			pos = save;
			//if (stopped == true)return 0;
			continue;
		}

		int score = -quiescence(-beta, -alpha);
		pos = save;

		--ply;
		//if (stopped == true)return 0;

		if (score >= beta) {
			//fail high
			//historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)] -= 1;
			//delete save;
			delete moveList;
			return beta;
		}

		if (score > alpha) {
			// new node
			//historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)] += 1;
			alpha = eeval;
		}
	}
	//delete save;
	delete moveList;
	return alpha;
}

inline int Game::negaMax(int alpha, int beta, int depth, bool pv, double nulled) {
	//if ((nodes & 2047) == 0) communicate();
	if (ply && isRepetition())return 0;

	int hashFlag = hashALPHA;
	int score;
	if (nulled > 0)nulled -= 0.5;


	bool pvNode = (beta - alpha) > 1;

	if (!pv && ply && (score = readHashEntry(pos.hashKey,alpha, beta, depth)) != 100000)
		return score;

	if (ply > maxPly - 1) {
		return eval();
	}
	pvLen[ply] = ply;
	if (depth <= 0)return quiescence(alpha,beta);

	//remove for spEEd
	
	
	++nodes;
	unsigned long kingPos;
	bitScanForward(&kingPos, (pos.side ? pos.bitboards[k] : pos.bitboards[K]));
	int inCheck = pos.isSquareAttacked(kingPos, pos.side ^ 1);
	if (inCheck) ++depth;
	// number of moves searched in a move list
	int moveSearched = 0;

	// evaluation pruning / static null move pruning
	int staticEval = eval();

	
	if (depth < 3 && !pv && !inCheck && abs(beta - 1) > -infinity + 100) {
		// define evaluation margin
		int evalMargin = 120 * depth;

		// evaluation margin substracted from static evaluation score fails high
		if (staticEval - evalMargin >= beta)
			// evaluation margin substracted from static evaluation score
			return staticEval - evalMargin;
	}

	//Null MP

	if (!pv && nulled <= 4 && depth >= 3 && inCheck == 0 && ply) {
		++ply;
		//++repetitionIndex;
		//repetitionTable[repetitionIndex] = pos.hashKey;

		repetitionIndex++;
		repetitionTable[repetitionIndex] = pos.hashKey;

		pos.side ^= 1;
		int exPassant = pos.enPassant;
		pos.enPassant = no_square;
		pos.hashKey ^= sideKeys;
		pos.hashKey ^= enPassantKeys[exPassant];
		int R = 2;
		R += depth > 3;
		R += depth > 5;
		R += depth > 7;
		R = min(R, 5 - nulled);
		int sscore = -negaMax(-beta, -beta + 1, depth - 1 - R, false, nulled + R);
		--ply;
		--repetitionIndex;

		pos.side ^= 1;
		pos.enPassant = exPassant;
		pos.hashKey ^= sideKeys;
		pos.hashKey ^= enPassantKeys[exPassant];

		//if (stopped == true)return 0;
		if (sscore >= beta) {
			return beta;
			depth -= R;
			if (depth <= 0)return quiescence(alpha, beta);
		}
	}

	

	
	// razoring
	if (!pv && !inCheck && depth <= 3)
	{
		// get static eval and add first bonus
		score = staticEval + 125;

		// define new score
		int newScore;

		// static evaluation indicates a fail-low node
		if (score < beta)
		{
			// on depth 1
			if (depth == 1)
			{
				// get quiscence score
				newScore = quiescence(alpha, beta);

				// return quiescence score if it's greater then static evaluation score
				return (newScore > score) ? newScore : score;
			}

			// add second bonus to static evaluation
			score += 175;

			// static evaluation indicates a fail-low node
			if (score < beta && depth <= 2)
			{
				// get quiscence score
				newScore = quiescence(alpha, beta);

				// quiescence score indicates fail-low node
				if (newScore < beta)
					// return quiescence score if it's greater then static evaluation score
					return (newScore > score) ? newScore : score;
			}
		}
	}

	
	


	

	
	
	moves* moveList= new moves;
	generateMoves(moveList, inCheck);
	Position save = pos;

	tt* entry = getEntry(pos.hashKey);
	//if (entry->flags != hashALPHA) {
	for (int i = 1; i < moveList->count; ++i) {
		if (onlyMove(moveList->m[i]) == entry->move) {
			if (moveList->m[1] > 0xFFFFFFFF00000000) {
				moveList->m[0] = moveList->m[1];
				moveList->m[1] = moveList->m[i];
				moveList->m[i] = 0;
			}
			else {
				moveList->m[0] = moveList->m[i];
				moveList->m[i] = 0;
			}
			break;
		}
	}
	//}

	/*if (entry->key == pos.hashKey) {
		for (int index = 0; index < moveList->count; index++) {
			if (moveList->m[index] == entry->move) {
				moveList->m[0] = moveList->m[index];
				moveList->m[index] = 0;
			}
		}
	}*/
	/*
	moveInt bestMove = 0;
	//test bestmove first if lower depth already analyzed
	if (entry->key == pos.hashKey) {
		for (int i = 0; i < moveList->count; i++) {
			if (moveList->m[i] == entry->move) {
				moveInt currMove = entry->move;
				if (currMove == entry->move) {
					++ply;
					if (moveLegal(currMove) && makeMove(currMove, allMoves) == 0) {
						ply--;
						pos = *save;
						//if (stopped == true)return 0;
					}
					else {
						score = -negaMax(-beta, -alpha, depth - 1, pv, nulled);
						pos = *save;
						++moveSearched;
						--ply;


						if (score > alpha) {
							bestMove = currMove;
							hashFlag = hashEXACT;
							historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)] += depth;
							// new node
							alpha = score;
							//write to pvtable
							pvTable[ply][ply] = onlyMove(currMove);
							//copy move from deeper ply
							for (int j = ply + 1; j < pvLen[ply + 1]; ++j) {
								pvTable[ply][j] = pvTable[ply + 1][j];
							}
							//adjust PVLen
							pvLen[ply] = pvLen[ply + 1];
							if (score >= beta) {
								//fail high
								if (isCapture(currMove) == 0) {
									killerMoves[1][ply] = killerMoves[0][ply];
									killerMoves[0][ply] = currMove;

								}
								delete moveList;
								delete save;
								// store hash entry with the score equal to beta
								writeHashEntry(pos.hashKey, beta, depth, currMove, hashBETA);
								return beta;
							}
						}
					}
				}
				break;
			}
		}
	}
	*/

	//OTTIMIZZAZIONE : PREV POS IS THE SAME THROUGHT THE WHOLE FOR CYCLE, MAYBE NOT SAVING IN MAKEMOVE?
	moveInt currMove;
	moveInt bestMove;
	for (int i = 0; i < moveList->count; ++i) {
		 currMove = onlyMove(moveList->m[i]);
		//if (currMove == entry->move)continue;
		++ply;
		++repetitionIndex;
		repetitionTable[repetitionIndex] = pos.hashKey;

		if (makeMove(currMove)) {
			int score = alpha + 1;

			if (moveSearched == 0) score = -negaMax(-beta, -alpha, depth - 1, true, nulled);
			else {
				if (moveSearched >= fullDepthMoves && depth >= reductionLimit && okToReduce(currMove) && !inCheck) {
					if (moveSearched < overReduct) {
						bool badStory = (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
						//badStory += (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= 0;
						score = -negaMax(-alpha - 1, -alpha, depth - 2 - badStory, false, nulled);
					}
					else {

						bool badStory = (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
						//badStory += (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= 0;
						score = -negaMax(-alpha - 1, -alpha, (int)(depth * 0.667) - badStory - 1, false, nulled);
						if (score > alpha) {
							score = -negaMax(-alpha - 1, -alpha, depth - 2 - badStory, false, nulled);
						}

					}


				}
				//else score = alpha + 1;

				if (score > alpha) {

					score = -negaMax(-alpha - 1, -alpha, depth - 1, false, nulled);

					if ((score > alpha) && (score < beta)) score = -negaMax(-beta, -alpha, depth - 1, pv, nulled);

				}
			}

			//score = -negaMax(-beta, -alpha, depth - 1);

			pos = save;
			//if (stopped == true)return 0;
			++moveSearched;
			--ply;
			--repetitionIndex;



			if (score > alpha) {
				bestMove = currMove;
				hashFlag = hashEXACT;
				historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				// new node
				alpha = score;
				//write to pvtable
				pvTable[ply][ply] = onlyMove(currMove);
				//copy move from deeper ply
				for (int j = ply + 1; j < pvLen[ply + 1]; ++j) {
					pvTable[ply][j] = pvTable[ply + 1][j];
				}
				//adjust PVLen
				pvLen[ply] = pvLen[ply + 1];

				if (score >= beta) {
					//fail high
					if (isCapture(currMove) == 0) {
						killerMoves[1][ply] = killerMoves[0][ply];
						killerMoves[0][ply] = currMove;
						//CM history (?)
						counterMoves[getMoveSource(pos.lastMove)][getMoveTarget(pos.lastMove)] = bestMove;
					}
					//delete save;
					delete moveList;
					// store hash entry with the score equal to beta
					writeHashEntry(pos.hashKey, beta, depth, currMove, hashBETA);
					return beta;
				}
			}
		}
		else {
			--ply;
			--repetitionIndex;
			//if (stopped == true)return 0;
			pos = save;
		}
	}
	
	//delete save;
	delete moveList;

	if (moveSearched == 0) {
		if (inCheck) {
			writeHashEntry(pos.hashKey, alpha, depth, bestMove, hashFlag);
			return -mateValue + ply;
		}
		else {
			return 0;
		}
	}
	

	writeHashEntry(pos.hashKey, alpha, depth, bestMove, hashFlag);
	return alpha; //fails low
}

/*
inline int Game::negaMax(int alpha, int beta, int depth, bool pv, int nulled) {
	//Current node flag, we assume is lower bound
	int hashFlag = hashALPHA;
	//the score
	int score;
	//is this part of a scoutSearch?
	bool isScoutNode = (beta - alpha) <= 1;
	//if not a pv node, if in scout, if not root and if hashTable:
	if (isScoutNode && ply && (score = readHashEntry(pos.hashKey, alpha, beta, depth) != 100000))
		return score;
	//check for ply overflow
	if (ply > maxPly - 1) {
		return eval();
	}
	//searching this ply at least at ply pv len
	pvLen[ply] = ply;
	//if depth == 0, drop into Q-search
	if (depth == 0)return quiescence(alpha, beta);
	//we are searching a new node
	++nodes;
	//let's see if position is check, if it is we extend depth
	unsigned long kingPos;
	bitScanForward(&kingPos, (pos.side ? pos.bitboards[k] : pos.bitboards[K]));
	bool inCheck = pos.isSquareAttacked(kingPos, pos.side ^ 1);
	if (inCheck)++depth;

	//the number of moves we searched so far
	int moveSearched = 0;

	//Null MP

	if (nulled >= 4 && depth >= 3 && inCheck == 0 && ply) {
		ply++;
		pos.side ^= 1;
		int exPassant = pos.enPassant;
		pos.enPassant = no_square;
		pos.hashKey ^= sideKeys;
		pos.hashKey ^= enPassantKeys[exPassant];
		
		//R += depth > 4;
		//R += depth > 6;
		int sscore = -negaMax(-beta, -beta + 1, depth - 2, false, nulled + 2);
		ply--;
		pos.side ^= 1;
		pos.enPassant = exPassant;
		pos.hashKey ^= sideKeys;
		pos.hashKey ^= enPassantKeys[exPassant];

		//if (stopped == true)return 0;
		if (sscore >= beta) {
			if (depth < 2)return beta;
			depth -= 2;
			if (depth <= 0)return quiescence(alpha, beta);
		}
	}

	


	//generate moveList
	moves* moveList = new moves;
	generateMoves(moveList);
	//save position
	Position* save = new Position(pos);

	

	

	//iterate all the possible moves
	for (int i = 0; i < moveList->count; ++i) {
		//remove bonus from move to save it later
		//the move being iterated: we can save the eventual fail low move here
		moveInt currMove = onlyMove(moveList->m[i]);
		//if (currMove == entry->move)continue;
		//increase ply
		++ply;
		//play move
		if (makeMove(currMove, allMoves) == 0) {
			//if illegal, undo the move
			--ply;
			pos = *save;
			continue;
		}
		//lil trick
		int score = alpha + 1;
		//first node must be searched at full depth
		if (moveSearched == 0)score = -negaMax(-beta, -alpha, depth - 1, pv, nulled);
		else {
			//check conditions to apply LMR
			if (moveSearched >= fullDepthMoves && depth >= reductionLimit && okToReduce(currMove) && !inCheck) {
				//if not to prune super aggressively
				//if (moveSearched < overReduct) {
					//flag if move is especially poor
					bool badStory = (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
					//LMR score
					score = -negaMax(-alpha - 1, -alpha, depth - 2 - badStory , false, nulled);
				//}
				//REEEEE pruning
				//else {
					//flag if move is especially poor
					//bool badStory = (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
					//LMR score
					//score = -negaMax(-alpha - 1, -alpha, (int)(depth * 0.667) - badStory - 1, false, nulled);
					//TODO: maybe to add a research here (?)
					//if(score>alpha && score<beta)score = -negaMax(-alpha - 1, -alpha, depth - 2 - badStory, false, nulled);
				//}
			}

			//if still fails
			if (score > alpha) {
				//do normal scout search
				score = -negaMax(-alpha - 1, -alpha, depth - 1, false, nulled);
				//if move seemed ok
				if ((score > alpha) && (score < beta)) score = -negaMax(-beta, -alpha, depth - 1, pv, nulled);
			}
		}

		//undo move
		pos = *save;
		//one more move searched
		++moveSearched;
		//decrease ply
		--ply;

		//if move is good
		if (score > alpha) {
			//at least one good move
			hashFlag = hashEXACT;
			//update alpha
			alpha = score;
			//add to pvTable
			pvTable[ply][ply] = currMove;
			//update pvTable
			for (int j = ply + 1; j < pvLen[ply + 1]; ++j) {
				pvTable[ply][j] = pvTable[ply + 1][j];
			}
			//adjust pvLen
			pvLen[ply] = pvLen[ply + 1];
			//if it also fails high
			if (score >= beta) {
				//only quiet moves are killerMoves
				if (isCapture(currMove) == 0) {
					killerMoves[1][ply] = killerMoves[0][ply];
					killerMoves[0][ply] = currMove;
				}
				//delete save and moveList
				delete save;
				delete moveList;
				//store fail high on entry
				writeHashEntry(pos.hashKey, beta, depth, hashBETA);
				//return the fail high
				return beta;
			}
		}
	}

	//in the end, remove all de allocated studd
	delete save;
	delete moveList;

	//check if stalemate or checkMate
	if (moveSearched == 0) {
		//in check -> checkmate
		if (inCheck) {
			//store the mate value in TT (handled by the function)
			//writeHashEntry(pos.hashKey, alpha, depth, currMove, hashFlag);
			//the mate value
			return -mateValue + ply;
		}
		else {
			//store the stalemate value in TT
			// (?) writeHashEntry(pos.hashKey,alpha,depth,bestmove,hashFlag);
			//staleMate
			return 0;
		}
	}

	//if not stale/check mate, store into TT
	writeHashEntry(pos.hashKey, alpha, depth, hashFlag);
	//the best move we have found
	return alpha;
}

*/

U64 _perftDriver(int depth,Game *game)
{
	moves move_list[1];
	int i;
	U64 nodes = 0;

	Position save = game->pos;

	game->generateMoves(move_list);
	for (i = 0; i <= move_list->count; i++) {
		if (game->makeMove(move_list->m[i])) {
			if (depth == 1) {
				++nodes;
				//game->print();
				//getchar();
			}
			
			else nodes += _perftDriver(depth - 1, game);
		}
		game->pos = save;
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
void printPosition(Position pos) {
	// loop over board ranks 
	for (int rank = 0; rank < 8; rank++) {
		//loop over files
		for (int file = 0; file < 8; file++) {
			int square = rank * 8 + file;
			int piece = -1; //no piece default
			if (!file)std::cout << 8 - rank << " "; //print ranks
			for (int test = 0; test < 12; test++) {
				if (testBit(pos.bitboards[test], square))piece = test;
			};
			std::cout << " " << ((piece == -1) ? '.' : asciiPieces[piece]);
		}
		std::cout << "\n";
	}
	std::cout << "\n   a b c d e f g h\n\n";

	//print side
	if (pos.side >= 0)std::cout << "   Side to move:     "<<(pos.side?" ":"") << ((!pos.side) ? "white" : "black") << "\n";
	else std::cout <<	"    Side to move unset! " << "\n";

	//print optional en croissant
	if (pos.enPassant != no_square) std::cout << "   En passant square:   " << coords[pos.enPassant] << "\n";
	else std::cout <<	"   No en passant square" << "\n";

	//print castling rights
	std::cout << "   Castling rights:   " << ((pos.castle & wk) ? "K" : "-") << ((pos.castle & wq) ? "Q" : "-") << ((pos.castle & bk) ? "k" : "-") << ((pos.castle & bq) ? "q" : "-") << "\n\n";
	
	//print hash key
	std::cout << "   Hash key: " << pos.hashKey << "\n";
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

	//reset rights to none
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
	newKey();
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
inline bool Position::isSquareAttacked(unsigned int square, int attackerSide) {
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
	memcpy(this, &other, sizeof(Position));
	/*memcpy(bitboards, other.bitboards, 96);
	memcpy(occupancies, other.occupancies, 24);
	side = other.side;
	enPassant = other.enPassant;
	castle = other.castle;*/
}

inline int Position::whiteCaptureValueAt(int square) {
	U64 sqBB = squareBB(square);
	if (bitboards[p] & sqBB) return 0x1ffffff;
	if (bitboards[n] & sqBB) return 0x3ffffff;
	if (bitboards[b] & sqBB) return 0x4ffffff;
	if (bitboards[r] & sqBB) return 0x5ffffff;
	return 0x9ffffff;
}

inline int Position::blackCaptureValueAt(int square) {
	U64 sqBB = squareBB(square);
	if (bitboards[P] & sqBB) return 0x1ffffff;
	if (bitboards[N] & sqBB) return 0x3ffffff;
	if (bitboards[B] & sqBB) return 0x4ffffff;
	if (bitboards[R] & sqBB) return 0x5ffffff;
	return 0x9ffffff;
}


#define kingCaptures   0x6fffff
#define pawnCaptures   0x5fffff
#define knightCaptures 0x3fffff
#define bishopCaptures 0x3fffff
#define rookCaptures   0x2fffff
#define queenCaptures  0x1fffff

#define queenPromotion  0x9fffff
#define knightPromotion 0x5fffff
#define rookPromotion   0x4fffff
#define bishopPromotion 0x3fffff

inline void Position::generateMoves(moves* moveList, bool isCheck) {
	//reset the moveList
	moveList->count = 0;
	U64 notWhite = ~(occupancies[white]);
	U64 notBlack = ~(occupancies[black]);
	U64 noneOccupancy = notWhite & notBlack;
	//unsigned long otherKing;
	//bitScanForward(&otherKing, bitboards[K + (6 * side)]);
	//U64 bishopCheckers = getBishopAttacks(otherKing, occupancies[both]) & (noneOccupancy);
	//U64 rookCheckers = getRookAttacks(otherKing, occupancies[both]) & (noneOccupancy);
	//U64 knightCheckers = knightAttacks[otherKing] & noneOccupancy;
	//U64 pawnCheckers = pawnAttacks[1 - side][otherKing] & noneOccupancy;
	// 
	//WHITE
	if (side == white) {
		for (int piece = P; piece <= K; ++piece) {
			if (piece == P) {
				U64 bitboard = bitboards[P];
				
				while (bitboard) {
					//get next pawn
					unsigned long sourceSquare;
					bitScanForward(&sourceSquare, bitboard);
					unsigned long targetSquare;
					targetSquare = sourceSquare - 8;
					//quiet moves
					if (!testBit(occupancies[both], targetSquare)) {
						// normal push
						if (targetSquare > h8) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 0, 0, 0, 0), 0);
							//double pushes
							targetSquare -= 8;
							if (sourceSquare >= a2 && !testBit(occupancies[both], targetSquare)) {
								addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 0, 1, 0, 0), 4);
							}
						}
						//promotion pushes
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, Q, 0, 0, 0, 0), queenPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, R, 0, 0, 0, 0), knightPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, N, 0, 0, 0, 0), rookPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, B, 0, 0, 0, 0), bishopPromotion);
						}
					}

					//pawn attacks
					U64 attacks = pawnAttacks[white][sourceSquare] & occupancies[black];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture promotion
						if (targetSquare > h8) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + pawnCaptures);
						}
						//normal pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, Q, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + queenPromotion + pawnCaptures);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, N, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + knightPromotion + pawnCaptures);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, R, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + rookPromotion + pawnCaptures);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, B, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + bishopPromotion + pawnCaptures);
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[white][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, P, 0, 1, 0, 1, 0), 0x1ffff + pawnCaptures);
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
							addMove(moveList, encodeMove(e1, c1, K, 0, 0, 0, 0, 1), 8);
						}
					}
				}

				unsigned long sourceSquare, targetSquare;
				//only one king, no loop necessary
				bitScanForward(&sourceSquare, bitboards[K]);

				//init attacks
				U64 attacks = kingAttacks[sourceSquare] & occupancies[black];
				U64 quiets = kingAttacks[sourceSquare] & noneOccupancy;
				//while not all squares analyzed
				while (attacks) {
					bitScanForward(&targetSquare, attacks);
					//if (isSquareAttacked(targetSquare, black))continue;
					addMove(moveList, encodeMove(sourceSquare, targetSquare, K, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + kingCaptures);
					clearBit(attacks, targetSquare);
				}
				while (quiets) {
					bitScanForward(&targetSquare, quiets);
					//if (isSquareAttacked(targetSquare, black))continue;
					addMove(moveList, encodeMove(sourceSquare, targetSquare, K, 0, 1, 0, 0, 0), (isCheck * 100));
					clearBit(quiets, targetSquare);
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
					U64 attacks = knightAttacks[sourceSquare] & occupancies[black];
					U64 quiets = knightAttacks[sourceSquare] & noneOccupancy;
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + knightCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 0, 0, 0, 0), 3 );
						clearBit(quiets, targetSquare);
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
					U64 attacks = getBishopAttacks(sourceSquare, occupancies[both]) & notWhite;
					U64 quiets = attacks & noneOccupancy;
					attacks ^= quiets;
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + bishopCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 0, 0, 0, 0), 3 );
						clearBit(quiets, targetSquare);
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
					U64 attacks = getRookAttacks(sourceSquare, occupancies[both]) & notWhite;
					U64 quiets = attacks & noneOccupancy;
					attacks ^= quiets;

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + rookCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 0, 0, 0, 0), 2 );
						clearBit(quiets, targetSquare);
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
					U64 attacks = getQueenAttacks(sourceSquare, occupancies[both]) & notWhite;
					U64 quiets = attacks & noneOccupancy;
					attacks ^= quiets;

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + queenCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 0, 0, 0, 0), 1);
						clearBit(quiets, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}
	//BLACK
	else {
		for (int piece = p; piece <= k; ++piece) {
			if (piece == p) {
				U64 bitboard = bitboards[p];
				
				while (bitboard) {
					//get next pawn
					unsigned long sourceSquare;
					bitScanForward(&sourceSquare, bitboard);
					unsigned long targetSquare;
					targetSquare = sourceSquare + 8;
					//quiet moves
					if (!testBit(occupancies[both], targetSquare)) {
						//normal push
						if (targetSquare <a1) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 0, 0, 0, 0), 0);
							//double pushes
							targetSquare += 8;
							if (sourceSquare <= h7 && !testBit(occupancies[both], targetSquare)) {
								addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 0, 1, 0, 0), 4);
							}
						}
						//promotion pushes
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 0, 0, 0, 0), queenPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 0, 0, 0, 0), knightPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 0, 0, 0, 0), rookPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 0, 0, 0, 0), bishopPromotion);
							
						}
					}

					//pawn attacks
					U64 attacks = pawnAttacks[black][sourceSquare] & occupancies[white];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture
						if (targetSquare < a1) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + pawnCaptures);
						}
						//promotion pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + queenPromotion + pawnCaptures);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + knightPromotion + pawnCaptures);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + rookPromotion + pawnCaptures);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + bishopPromotion + pawnCaptures);
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[black][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, p, 0, 1, 0, 1, 0), 0x1ffff + pawnCaptures);
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
				U64 attacks = kingAttacks[sourceSquare] & occupancies[white];
				U64 quiets = kingAttacks[sourceSquare] & noneOccupancy;
				//while not all squares analyzed
				while (attacks) {
					bitScanForward(&targetSquare, attacks);
					//if (isSquareAttacked(targetSquare, black))continue;
					addMove(moveList, encodeMove(sourceSquare, targetSquare, k, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + kingCaptures);
					clearBit(attacks, targetSquare);
				}
				while (quiets) {
					bitScanForward(&targetSquare, quiets);
					//if (isSquareAttacked(targetSquare, black))continue;
					addMove(moveList, encodeMove(sourceSquare, targetSquare, k, 0, 1, 0, 0, 0), (isCheck * 100));
					clearBit(quiets, targetSquare);
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
					U64 attacks = knightAttacks[sourceSquare] & occupancies[white];
					U64 quiets = knightAttacks[sourceSquare] & noneOccupancy;

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + knightCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 0, 0, 0, 0), 3);
						clearBit(quiets, targetSquare);
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
					U64 attacks = getBishopAttacks(sourceSquare, occupancies[both]) & notBlack;
					U64 quiets = attacks & noneOccupancy;
					attacks ^= quiets;
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + bishopCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 0, 0, 0, 0), 3);
						clearBit(quiets, targetSquare);
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
					U64 attacks = getRookAttacks(sourceSquare, occupancies[both]) & notBlack;
					U64 quiets = attacks & noneOccupancy;
					attacks ^= quiets;

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + rookCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 0, 0, 0, 0), 2);
						clearBit(quiets, targetSquare);
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
					U64 attacks = getQueenAttacks(sourceSquare, occupancies[both]) & notBlack;
					U64 quiets = attacks & noneOccupancy;
					attacks ^= quiets;

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + queenCaptures);
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 0, 0, 0, 0), 1);
						clearBit(quiets, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}

	std::sort(std::begin(moveList->m), std::begin(moveList->m)+moveList->count, std::greater<U64>());
}

/*inline moveInt Game::IID(int depth) {
	//first, we generate a move list
	moves moveList;
	generateMoves(&moveList);
	unsigned char bestMove = 0;
	int score = -infinity;
	Position save = pos;
	for (int i = 0; i < moveList.count; ++i) {
		++ply;
		++repetitionIndex;
		repetitionTable[repetitionIndex] = pos.hashKey;
		if (makeMove(moveList.m[i])) {
			int currScore = -negaMax(-infinity, +infinity, depth, true, 0);
			--ply;
			--repetitionIndex;
			if (currScore > score) {
				bestMove = i;
				score = currScore;
			}
		}
		else {
			--ply;
			--repetitionIndex;
		}
		pos = save;
	}
	return moveList.m[bestMove];
}*/

inline void Position::generateCaptures(moves* moveList) {
	//reset the moveList
	moveList->count = 0;
	U64 notWhite = ~(occupancies[white]);
	U64 notBlack = ~(occupancies[black]);
	U64 noneOccupancy = notWhite & notBlack;
	//WHITE
	if (side == white) {
		for (int piece = P; piece <= K; ++piece) {
			if (piece == P) {
				U64 bitboard = bitboards[P];

				while (bitboard) {
					//get next pawn
					unsigned long sourceSquare;
					bitScanForward(&sourceSquare, bitboard);
					unsigned long targetSquare;
					targetSquare = sourceSquare - 8;
					//pawn attacks
					U64 attacks = pawnAttacks[white][sourceSquare] & occupancies[black];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture promotion
						if (targetSquare > h8) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 1, 0, 0, 0), pawnCaptures + whiteCaptureValueAt(targetSquare));
						}
						//normal pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, Q, 1, 0, 0, 0), pawnCaptures + whiteCaptureValueAt(targetSquare) + queenPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, R, 1, 0, 0, 0), pawnCaptures + whiteCaptureValueAt(targetSquare) + rookPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, N, 1, 0, 0, 0), pawnCaptures + whiteCaptureValueAt(targetSquare) + knightPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, B, 1, 0, 0, 0), pawnCaptures + whiteCaptureValueAt(targetSquare) + bishopPromotion);
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[white][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, P, 0, 1, 0, 1, 0), pawnCaptures + 0x1ffff);
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
							addMove(moveList, encodeMove(e1, g1, K, 0, 0, 0, 0, 1), 8);
						}
					}
				}
				if (castle & wq) {
					//free castling target squares
					if (!testBit(occupancies[both], d1) && !testBit(occupancies[both], c1) && !testBit(occupancies[both], b1)) {
						//traverse squares and king are not attacked
						if (!isSquareAttacked(e1, black) && !isSquareAttacked(d1, black) && !isSquareAttacked(c1, black)) {
							addMove(moveList, encodeMove(e1, c1, K, 0, 0, 0, 0, 1), 8);
						}
					}
				}

				unsigned long sourceSquare, targetSquare;
				//only one king, no loop necessary
				bitScanForward(&sourceSquare, bitboards[K]);

				//init attacks
				U64 attacks = kingAttacks[sourceSquare] & occupancies[black];
				//while not all squares analyzed
				while (attacks) {
					bitScanForward(&targetSquare, attacks);
					//if (isSquareAttacked(targetSquare, black))continue;
					addMove(moveList, encodeMove(sourceSquare, targetSquare, K, 0, 1, 0, 0, 0), kingCaptures + whiteCaptureValueAt(targetSquare));
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
					U64 attacks = knightAttacks[sourceSquare] & occupancies[black];
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 1, 0, 0, 0), knightCaptures + whiteCaptureValueAt(targetSquare) );
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
					U64 attacks = getBishopAttacks(sourceSquare, occupancies[both]) & occupancies[black];
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 1, 0, 0, 0), bishopCaptures + whiteCaptureValueAt(targetSquare));
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
					U64 attacks = getRookAttacks(sourceSquare, occupancies[both]) & occupancies[black];
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 1, 0, 0, 0), rookCaptures + whiteCaptureValueAt(targetSquare) );
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
					U64 attacks = getQueenAttacks(sourceSquare, occupancies[both]) & occupancies[black];
					
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 1, 0, 0, 0), queenCaptures + whiteCaptureValueAt(targetSquare) );
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}
	//BLACK
	else {
		for (int piece = p; piece <= k; ++piece) {
			if (piece == p) {
				U64 bitboard = bitboards[p];

				while (bitboard) {
					//get next pawn
					unsigned long sourceSquare;
					bitScanForward(&sourceSquare, bitboard);
					unsigned long targetSquare;
					targetSquare = sourceSquare + 8;
					//pawn attacks
					U64 attacks = pawnAttacks[black][sourceSquare] & occupancies[white];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture
						if (targetSquare < a1) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 1, 0, 0, 0), pawnCaptures + blackCaptureValueAt(targetSquare));
						}
						//promotion pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 1, 0, 0, 0), pawnCaptures + blackCaptureValueAt(targetSquare) + queenPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 1, 0, 0, 0), pawnCaptures + blackCaptureValueAt(targetSquare) + rookPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 1, 0, 0, 0), pawnCaptures + blackCaptureValueAt(targetSquare) + knightPromotion);
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 1, 0, 0, 0), pawnCaptures + blackCaptureValueAt(targetSquare) + bishopPromotion);
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[black][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, p, 0, 1, 0, 1, 0), 0x1ffff + pawnCaptures);
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
							addMove(moveList, encodeMove(e8, g8, k, 0, 0, 0, 0, 1), 10);
						}
					}
				}
				if (castle & bq) {
					//free castling target squares
					if (!testBit(occupancies[both], d8) && !testBit(occupancies[both], c8) && !testBit(occupancies[both], b8)) {
						//traverse squares and king are not attacked
						if (!isSquareAttacked(e8, white) && !isSquareAttacked(d8, white) && !isSquareAttacked(c8, white)) {
							addMove(moveList, encodeMove(e8, c8, k, 0, 0, 0, 0, 1), 10);
						}
					}
				}

				unsigned long sourceSquare, targetSquare;
				//only one king, no loop necessary
				bitScanForward(&sourceSquare, bitboards[k]);

				//init attacks
				U64 attacks = kingAttacks[sourceSquare] & occupancies[white];
				//while not all squares analyzed
				while (attacks) {
					bitScanForward(&targetSquare, attacks);
					//if (isSquareAttacked(targetSquare, black))continue;
					addMove(moveList, encodeMove(sourceSquare, targetSquare, k, 0, 1, 0, 0, 0), kingCaptures + blackCaptureValueAt(targetSquare) );
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
					U64 attacks = knightAttacks[sourceSquare] & occupancies[white];
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 1, 0, 0, 0), knightCaptures + blackCaptureValueAt(targetSquare));
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
					U64 attacks = getBishopAttacks(sourceSquare, occupancies[both]) & occupancies[white];
					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 1, 0, 0, 0), bishopCaptures + blackCaptureValueAt(targetSquare));
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
					U64 attacks = getRookAttacks(sourceSquare, occupancies[both]) & occupancies[white];

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 1, 0, 0, 0), rookCaptures + blackCaptureValueAt(targetSquare));
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
					U64 attacks = getQueenAttacks(sourceSquare, occupancies[both]) & occupancies[white];

					//loop over target squares
					while (attacks) {
						bitScanForward(&targetSquare, attacks);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 1, 0, 0, 0), queenCaptures + blackCaptureValueAt(targetSquare));
						clearBit(attacks, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}
	std::sort(std::begin(moveList->m),std::begin(moveList->m) + moveList->count, std::greater<U64>());
}

inline void Position::newKey() {
	hashKey = 0;
	for (int i = 0; i < 12; i++) {
		U64 bitboard = bitboards[i];
		while (bitboard) {
			unsigned long square;
			bitScanForward(&square, bitboard);
			clearBit(bitboard, square);
			hashKey ^= pieceKeys[i][square];
		}
	}
	if (enPassant != no_square) hashKey ^= enPassantKeys[enPassant];
	hashKey ^= castleKeys[castle];
	if (side)hashKey ^= sideKeys;
}

void Game::generateMoves(moves* moveList, bool isCheck) {
	pos.generateMoves(moveList,isCheck);
}

void Game::generateCaptures(moves* moveList) {
	pos.generateCaptures(moveList);
}
void Game::generateLegalMoves(moves* moveList) { 
	pos.generateMoves(moveList);
}
/*
inline int Game::makeMove(moveInt move) {
	if (!move)return 0;
	int source = getMoveSource(move);
	int target = getMoveTarget(move);
	int piece = getMovePiece(move);


	//move the piece

	setBit(pos.bitboards[piece], target);
	clearBit(pos.bitboards[piece], source);

	//hash update
	//remove piece from source and put it on target
	pos.hashKey ^= pieceKeys[piece][source];
	pos.hashKey ^= pieceKeys[piece][target];

	//handle capture
#define clearMode 0
#if clearMode == 0
	if (getCaptureFlag(move)) {
		if (pos.side == white) {
			if (testBit(pos.bitboards[6], target)) {
				//if piece, remove
				clearBit(pos.bitboards[6], target);
				pos.hashKey ^= pieceKeys[6][target];
				goto NEXT;
			}
			else if (testBit(pos.bitboards[7], target)) {
				//if piece, remove
				clearBit(pos.bitboards[7], target);
				pos.hashKey ^= pieceKeys[7][target];
				goto NEXT;
			}
			else if (testBit(pos.bitboards[8], target)) {
				//if piece, remove
				clearBit(pos.bitboards[8], target);
				pos.hashKey ^= pieceKeys[8][target];
				goto NEXT;
			}
			else if (testBit(pos.bitboards[9], target)) {
				//if piece, remove
				clearBit(pos.bitboards[9], target);
				pos.hashKey ^= pieceKeys[9][target];
				goto NEXT;
			}
			//if piece, remove
			clearBit(pos.bitboards[10], target);
			pos.hashKey ^= pieceKeys[10][target];
		}
		else {
			if (testBit(pos.bitboards[0], target)) {
				//if piece, remove
				clearBit(pos.bitboards[0], target);
				pos.hashKey ^= pieceKeys[0][target];
				goto NEXT;
			}
			else if (testBit(pos.bitboards[1], target)) {
				//if piece, remove
				clearBit(pos.bitboards[1], target);
				pos.hashKey ^= pieceKeys[1][target];
				goto NEXT;
			}
			else if (testBit(pos.bitboards[2], target)) {
				//if piece, remove
				clearBit(pos.bitboards[2], target);
				pos.hashKey ^= pieceKeys[2][target];
				goto NEXT;
			}
			else if (testBit(pos.bitboards[3], target)) {
				//if piece, remove
				clearBit(pos.bitboards[3], target);
				pos.hashKey ^= pieceKeys[3][target];
				goto NEXT;
			}
			//if piece, remove
			clearBit(pos.bitboards[4], target);
			pos.hashKey ^= pieceKeys[4][target];
		}
	}
#else
		//if (getCaptureFlag(move)) {
	if (pos.side) {
		clearBit(pos.bitboards[0], target);
		clearBit(pos.bitboards[1], target);
		clearBit(pos.bitboards[2], target);
		clearBit(pos.bitboards[3], target);
		clearBit(pos.bitboards[4], target);
	}
	else {
		clearBit(pos.bitboards[6], target);
		clearBit(pos.bitboards[7], target);
		clearBit(pos.bitboards[8], target);
		clearBit(pos.bitboards[9], target);
		clearBit(pos.bitboards[10], target);
	}
	//}
#endif

		//handle promotion
NEXT: int promotion = getPromotion(move);
	if (promotion) {
		//erase pawn from target square

		clearBit(pos.bitboards[(pos.side ? 6 : 0)], target);
		//removing pawn and adding promotion
		pos.hashKey ^= pieceKeys[piece][target];
		pos.hashKey ^= pieceKeys[promotion][target];
		//set promoted piece
		setBit(pos.bitboards[promotion], target);
	}

	//handling croissants
	if (getEnPassantFlag(move)) {
		(pos.side) ? clearBit(pos.bitboards[P], target - 8) : clearBit(pos.bitboards[p], target + 8);
	}

	//remove enpassant (?)
	pos.hashKey ^= enPassantKeys[pos.enPassant];
	//handle double pushes (updating croissants)
	if (getDoubleFlag(move)) {
		int enPass = (pos.side) ? (target - 8) : (target + 8);
		pos.hashKey ^= enPassantKeys[enPass];
		pos.enPassant = enPass;
	}
	else {
		//reset croissants
		pos.enPassant = no_square;
	}

	//handling castle
	if (getCastleFlag(move)) {
		switch (target) {
		case c1:
			clearBit(pos.bitboards[R], a1);
			setBit(pos.bitboards[R], d1);
			//hash castle rook
			pos.hashKey ^= pieceKeys[R][a1];
			pos.hashKey ^= pieceKeys[R][d1];
			break;

		case g1:
			clearBit(pos.bitboards[R], h1);
			setBit(pos.bitboards[R], f1);
			//hash castle rook
			pos.hashKey ^= pieceKeys[R][h1];
			pos.hashKey ^= pieceKeys[R][f1];
			break;

		case c8:
			clearBit(pos.bitboards[r], a8);
			setBit(pos.bitboards[r], d8);
			//hash castle rook
			pos.hashKey ^= pieceKeys[R][a8];
			pos.hashKey ^= pieceKeys[R][d8];
			break;

		case g8:
			clearBit(pos.bitboards[r], h8);
			setBit(pos.bitboards[r], f8);
			//hash castle rook
			pos.hashKey ^= pieceKeys[R][h8];
			pos.hashKey ^= pieceKeys[R][f8];
			break;
		}
	}

	//remove hash
	pos.hashKey ^= castleKeys[pos.castle];
	//calculate new castle
	pos.castle &= castlingRights[source];
	pos.castle &= castlingRights[target];
	//put new castle
	pos.hashKey ^= castleKeys[pos.castle];



	pos.occupancies[0] = pos.bitboards[0] | pos.bitboards[1] | pos.bitboards[2] | pos.bitboards[3] | pos.bitboards[4] | pos.bitboards[5];
	pos.occupancies[1] = pos.bitboards[6] | pos.bitboards[7] | pos.bitboards[8] | pos.bitboards[9] | pos.bitboards[10] | pos.bitboards[11];
	pos.occupancies[2] = pos.occupancies[1] | pos.occupancies[0];

	// italy moment
	pos.side ^= 1;
	pos.hashKey ^= sideKeys;

	// make sure that king has not been exposed into a check
	unsigned long king = 0ULL;
	(pos.side) ? bitScanReverse(&king, pos.bitboards[K]) : bitScanForward(&king, pos.bitboards[k]);


	if (pos.isSquareAttacked(king, pos.side))
	{
		// return illegal move
		return 0;
	}


	//pos.side ^= 1;
	//nice move

	//positionStack.push(pos);
	//saveState();
	pos.lastMove = move;
	return 1;

}*/

inline int Game::makeMove(moveInt move, int flags) {
	//std::cout << "call\n";
	//allMove makeMove
	if (!move) return 0;
	if (flags == allMoves) {

	
		
		int source = getMoveSource(move);
		int target = getMoveTarget(move);
		int piece = getMovePiece(move);
		

		//move the piece
		
		setBit(pos.bitboards[piece], target);
		clearBit(pos.bitboards[piece], source);

		//hash update
		//remove piece from source and put it on target
		pos.hashKey ^= pieceKeys[piece][source];
		pos.hashKey ^= pieceKeys[piece][target];

		//handle capture
#define clearMode 0
#if clearMode == 0
		if (getCaptureFlag(move)) {
			if (pos.side == white) {	
				if (testBit(pos.bitboards[6], target)) {
					//if piece, remove
					clearBit(pos.bitboards[6], target);
					pos.hashKey ^= pieceKeys[6][target];
					goto NEXT;
				}
				else if (testBit(pos.bitboards[7], target)) {
					//if piece, remove
					clearBit(pos.bitboards[7], target);
					pos.hashKey ^= pieceKeys[7][target];
					goto NEXT;
				}
				else if (testBit(pos.bitboards[8], target)) {
					//if piece, remove
					clearBit(pos.bitboards[8], target);
					pos.hashKey ^= pieceKeys[8][target];
					goto NEXT;
				}
				else if (testBit(pos.bitboards[9], target)) {
					//if piece, remove
					clearBit(pos.bitboards[9], target);
					pos.hashKey ^= pieceKeys[9][target];
					goto NEXT;
				}
					//if piece, remove
					clearBit(pos.bitboards[10], target);	
					pos.hashKey ^= pieceKeys[10][target];
			}
			else {
				if (testBit(pos.bitboards[0], target)) {
					//if piece, remove
					clearBit(pos.bitboards[0], target);
					pos.hashKey ^= pieceKeys[0][target];
					goto NEXT;
				}
				else if (testBit(pos.bitboards[1], target)) {
					//if piece, remove
					clearBit(pos.bitboards[1], target);
					pos.hashKey ^= pieceKeys[1][target];
					goto NEXT;
				}
				else if (testBit(pos.bitboards[2], target)) {
					//if piece, remove
					clearBit(pos.bitboards[2], target);
					pos.hashKey ^= pieceKeys[2][target];
					goto NEXT;
				}
				else if (testBit(pos.bitboards[3], target)) {
					//if piece, remove
					clearBit(pos.bitboards[3], target);
					pos.hashKey ^= pieceKeys[3][target];
					goto NEXT;
				}
				//if piece, remove
				clearBit(pos.bitboards[4], target);
				pos.hashKey ^= pieceKeys[4][target];
			}
		}
#else
		//if (getCaptureFlag(move)) {
			if (pos.side) {
				clearBit(pos.bitboards[0], target);
				clearBit(pos.bitboards[1], target);
				clearBit(pos.bitboards[2], target);
				clearBit(pos.bitboards[3], target);
				clearBit(pos.bitboards[4], target);
			}
			else {
				clearBit(pos.bitboards[6], target);
				clearBit(pos.bitboards[7], target);
				clearBit(pos.bitboards[8], target);
				clearBit(pos.bitboards[9], target);
				clearBit(pos.bitboards[10], target);
			}
		//}
#endif
		
		//handle promotion
	NEXT: int promotion = getPromotion(move);
		if (promotion) {
			//erase pawn from target square
			
			clearBit(pos.bitboards[(pos.side?6:0)], target);
			//removing pawn and adding promotion
			pos.hashKey ^= pieceKeys[piece][target];
			pos.hashKey ^= pieceKeys[promotion][target];
			//set promoted piece
			setBit(pos.bitboards[promotion], target);
		}

		//handling croissants
		if (getEnPassantFlag(move)) {
			(pos.side) ? clearBit(pos.bitboards[P], target - 8) : clearBit(pos.bitboards[p], target + 8);
			pos.hashKey ^= (pos.side) ? pieceKeys[P][target - 8] : pieceKeys[p][target + 8];
		}

		//remove enpassant (?)
		pos.hashKey ^= enPassantKeys[pos.enPassant];
		//handle double pushes (updating croissants)
		if (getDoubleFlag(move)) {
			int enPass = (pos.side) ? (target - 8) : (target + 8);
			pos.hashKey ^= enPassantKeys[enPass];
			pos.enPassant = enPass;
		}
		else {
			//reset croissants
			pos.enPassant = no_square;
		}

		//handling castle
		if (getCastleFlag(move)) {
			switch (target) {
			case c1:
				clearBit(pos.bitboards[R], a1);
				setBit(pos.bitboards[R], d1);
				//hash castle rook
				pos.hashKey ^= pieceKeys[R][a1];
				pos.hashKey ^= pieceKeys[R][d1];
				break;

			case g1:
				clearBit(pos.bitboards[R], h1);
				setBit(pos.bitboards[R], f1);
				//hash castle rook
				pos.hashKey ^= pieceKeys[R][h1];
				pos.hashKey ^= pieceKeys[R][f1];
				break;

			case c8:
				clearBit(pos.bitboards[r], a8);
				setBit(pos.bitboards[r], d8);
				//hash castle rook
				pos.hashKey ^= pieceKeys[r][a8];
				pos.hashKey ^= pieceKeys[r][d8];
				break;

			case g8:
				clearBit(pos.bitboards[r], h8);
				setBit(pos.bitboards[r], f8);
				//hash castle rook
				pos.hashKey ^= pieceKeys[r][h8];
				pos.hashKey ^= pieceKeys[r][f8];
				break;
			}
		}
		
		//remove hash
		pos.hashKey ^= castleKeys[pos.castle];
		//calculate new castle
		pos.castle &= castlingRights[source];
		pos.castle &= castlingRights[target];
		//put new castle
		pos.hashKey ^= castleKeys[pos.castle];

		

		pos.occupancies[0] = pos.bitboards[0] | pos.bitboards[1] | pos.bitboards[2] | pos.bitboards[3] | pos.bitboards[4] | pos.bitboards[5];
		pos.occupancies[1] = pos.bitboards[6] | pos.bitboards[7] | pos.bitboards[8] | pos.bitboards[9] | pos.bitboards[10] | pos.bitboards[11];
		pos.occupancies[2]  = pos.occupancies[1] | pos.occupancies[0];

		// italy moment
		pos.side ^= 1;
		pos.hashKey ^= sideKeys;
		
		// make sure that king has not been exposed into a check
		unsigned long king = 0ULL;
		(pos.side) ? bitScanReverse(&king, pos.bitboards[K]) : bitScanForward(&king, pos.bitboards[k]);
		

		if (pos.isSquareAttacked(king,pos.side))
		{
			// return illegal move
			return 0;
		}


		//pos.side ^= 1;
		//nice move
		
		//positionStack.push(pos);
		//saveState();
		pos.lastMove = move;
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
	Position ps;
	ps.parseFen(startPosition);
	pos = ps;
	positionStack.emplace(ps);
};

Game::Game(Position ps) {
	nodes = 0;
	ply = 0;
	pos = ps;
	positionStack.emplace(ps);
}

Game::Game(const char* fen) {
	nodes = 0;
	ply = 0;
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
	for (int i = 0; i < moveList->count; ++i) {
		if (!strcmp(getMoveString(moveList->m[i]).c_str(), moveString))return true;
	}
	return false;
}

moveInt Game::getLegal(const char* moveString) {
	//create moveList instance
	moves moveList[1];
	//generate moves
	generateMoves(moveList);
	for (int i = 0; i < moveList->count; ++i) {
		if (!strcmp(getMoveString(moveList->m[i]).c_str(), moveString))return moveList->m[i];
	}
	return false;
}

inline void Position::addMove(moves* moveList, moveInt move, int bonus) {

	if (pvTable[0][ply] == move) {
		move |= (0xfffffff000000);
	}
	else if (killerMoves[0][ply] == move) {
		move |= (0xeffff000000);
	}
	else if (killerMoves[1][ply] == move) {
		move |= (0xdffff000000);
	}
	else if (move == counterMoves[getMoveSource(lastMove)][getMoveTarget(lastMove)]) {
		move |= (0xfffff000000);
	}
	else {
#define centerBonus 10 //10 > 7 > 2 > 16
		bonus += historyMoves[getMovePiece(move)][getMoveTarget(move)]>>1;
		bonus += centerBonusTable10[getMoveTarget(move)];
		
		move |= ((U64)bonus << 24);
	}

	moveList->m[moveList->count] = move;
	++moveList->count;


}