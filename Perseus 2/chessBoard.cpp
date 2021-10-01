#pragma once
#include "chessBoard.h"
#include "evaluation.h"
#include "pestoEval.h"
#include <windows.h>
#include "uci.h"

moveInt killerMoves[2][maxPly] = { {0} };
int historyMoves[maxPly][12][64] = { {0	} };
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

U64 getTimeMs() {
	return GetTickCount64();
}

inline unsigned long long perftDriver(int depth,const char *fen=startPosition) {
	U64 nodes = 0;
	U64 start = getTimeMs();
	Game game(fen);
	game.print();
	nodes = _perftDriver(depth, &game);
	U64 end = getTimeMs();
	std::cout << nodes << " nodes found at depth " << depth << " in " << end - start << " ms.\n";
	return nodes;
}

inline unsigned long long perftDriver(int depth, Game *game) {
	U64 nodes = 0;
	U64 start = getTimeMs();
	game->print();
	nodes = _perftDriver(depth, game);
	U64 end = getTimeMs();
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
#define VALWINDOW 50
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
	std::string bestMove = "";
	U64 totnodes = 0;
	startTime = getTimeMs();
	std::cout << "startTime is " << startTime << "\n";
	moveTime = startTime + calcMoveTime();
	std::cout << "End time is" << moveTime << "\n";
	for (int c = 3; c <= depth; ++c) {
		if (stopped == true) {
			stopped = false;
			break;
		}
		nodes = 0;
		//enable followPv
		//
		std::cout << "Window of search: " << alpha << " " << beta << "\n";
		U64 timer = getTimeMs();
		
		
		
		if (!stopped)score = negaMax(alpha, beta, c);
		else goto NEXT;

		if (alpha != -50000 && beta != 50000) {
			for (int i = 4; i <= 16; i*=2) {
				if (score <= alpha) alpha -= VALWINDOW * i;
				else if (score >= beta) beta += VALWINDOW * i;
				else { break; }
				std::cout << "(Research) Window of search: " << alpha << " " << beta << "\n";
				if(!stopped)score = negaMax(alpha, beta, c);
				else goto NEXT;
			}
		}
		
		if(!stopped) if (score <= alpha || score >= beta) {
			score = negaMax(-50000, 50000, c);
		}
		U64 time2 = getTimeMs();
		if (!stopped) {
			alpha = score - VALWINDOW;
			beta = score + VALWINDOW;

			if (score<-mateScore && score >-mateValue) {
				std::cout << "info score mate " << -(score + mateValue) / 2 - 1 << " depth " << c << " nodes " << nodes << " pv ";
			}
			else if (score<mateValue && score >mateScore) {
				std::cout << "info score mate " << (mateValue - score) / 2 + 1 << " depth " << c << " nodes " << nodes << " pv ";
			}
			else {
				std::cout << "info score cp " << score << " depth " << c << " nodes " << nodes << " pv ";
			}

			for (int i = 0; i < pvLen[0]; ++i) {
				//historyMoves[getMovePiece(pvTable[0][i])][getMoveTarget(pvTable[0][i])] ++;
				std::cout << getMoveString(pvTable[0][i]) << " ";
			}
			std::cout << " speed " << (nodes / (time2 - timer + 1)) << "kN/S" << std::endl;

			bestMove = getMoveString(pvTable[0][0]);
		}
		else break;
		
		
		//if (abs(score) > 48000 && c >= 8)break;
		//if (c + 2 > depth && nodes < 1000000)depth += 2;
	}
NEXT: std::cout << "bestmove " << bestMove << std::endl;
	stopped =false;
	//wipeTT();

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
	
	if ((nodes & 2047) == 0) {
		communicate();
		if (stopped)return 0;
	}

	++nodes;
	unsigned long kingPos;
	bitScanForward(&kingPos, pos.bitboards[K + pos.side * 6]);
	int inCheck = pos.isSquareAttacked(kingPos, pos.side ^ 1);
	
	int eeval = eval();

	if (ply > maxPly - 1) {
		return eeval;
	}

	//fail hard beta
	if (eeval >= beta)return beta;

	//DELTA VALUE
#define BIGDELTA 775 // queen value
//if (isPromotingPawn()) BIG_DELTA += 775;

	if (!inCheck && eeval < alpha - BIGDELTA) {
		return alpha;
	}


	if (eeval > alpha)alpha = eeval;
	moves* moveList = new moves;
	(inCheck)?generateMoves(moveList):generateCaptures(moveList);
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
			// historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)] += 1;
			alpha = eeval;
		}
	}
	//delete save;
	delete moveList;
	return alpha;
}


inline int Game::negaMax2(int alpha, int beta, int depth, bool cutNode) {

	//Step 0: uci protocol
	if ((nodes & 2047) == 0) {
		communicate();
		if (stopped) return 0;
	}
	++nodes;
	//Step 1: check for draw by repetition and ply overflow
	if (ply && isRepetition()) return 0;
	if (ply > maxPly - 1) return eval();

	
	//Step 2: ttEntry search ?
	int score;
	if (cutNode && ply && (score = readHashEntry(pos.hashKey, alpha, beta, depth) != 100000)) return score;

	//Step 3: if depth <= 0, dive into quiescence
	if (depth <= 0)return quiescence(alpha, beta);

	//Step 4: flags & table updates
	int hashFlag = hashALPHA;
	pvLen[ply] = ply;

	unsigned long kingPos;
	bitScanForward(&kingPos, (pos.side ? pos.bitboards[k] : pos.bitboards[K]));
	int inCheck = pos.isSquareAttacked(kingPos, pos.side ^ 1);
	//Step 5: check extension
	if (inCheck) ++depth;

	int moveSearched = 0;
	int staticEval = eval();

	//Step 6: evaluation pruning
	if (depth < 3 && cutNode && !inCheck && abs(beta - 1) > -infinity + 100) {
		// define evaluation margin
		int evalMargin = 120 * depth;

		// evaluation margin substracted from static evaluation score fails high
		if (staticEval - evalMargin >= beta)
			// evaluation margin substracted from static evaluation score
			return staticEval - evalMargin;
	}

	//Step 7: Null MP
	if (depth >= 3 && inCheck == 0 && ply) {
		++ply;
		repetitionTable[++repetitionIndex] = pos.hashKey;
		int lastMove;
		lastMove = pos.lastMove;
		pos.side ^= 1;
		int exPassant = pos.enPassant;
		pos.enPassant = no_square;
		pos.hashKey ^= sideKeys;
		pos.hashKey ^= enPassantKeys[exPassant];
		
		int sscore = -negaMax(-beta, -beta + 1, depth - 1 - 2, false);
		--ply;
		--repetitionIndex;

		pos.lastMove = lastMove;
		pos.side ^= 1;
		pos.enPassant = exPassant;
		pos.hashKey ^= sideKeys;
		pos.hashKey ^= enPassantKeys[exPassant];

		//if (stopped == true)return 0;
		if (sscore >= beta) {
			//historyMoves[getMovePiece(lastMove)][getMoveTarget(lastMove)]++;
			return beta;
			depth -= R;
			if (depth <= 0)return quiescence(alpha, beta);
		}
	}

	//Step 8: razoring
	if (cutNode && !inCheck && depth < 3)
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

	//Step 9: move generation
	moves* moveList = new moves;
	generateMoves(moveList, inCheck);
	Position save = pos;

	//Step 10: insert tt move into moveList
	tt* entry = getEntry(pos.hashKey);
	//if (entry->flags != hashALPHA) {
	for (int i = 1; i < moveList->count; ++i) {
		if (onlyMove(moveList->m[i]) == entry->move) {
			if (moveList->m[1] > 0xFFFFFFFF00000000) {

				moveList->m[0] = moveList->m[1];
				moveList->m[1] = entry->move;
				moveList->m[i] = 0;
			}
			else {
				moveList->m[0] = entry->move;
				moveList->m[i] = 0;
			}
			break;
		}
	}
	//Step 10: IID (?)
	if (depth > 5) {
		if (moveList->m[0] == 0 && moveList->m[1] < 0xFFFFFFFF00000000) {
			if (abs(beta - alpha) > 1) moveList->m[0] = IID(moveList);
		}
	}

	//Step 11: Move iteration
	moveInt currMove;
	moveInt bestMove = 0;

	for (int i = 0; i < moveList->count; ++i) {
		currMove = onlyMove(moveList->m[i]);
		++ply;
		++repetitionIndex;
		repetitionTable[repetitionIndex] = pos.hashKey;

		if (makeMove(currMove)) {
			int score = alpha + 1;

			if (moveSearched == 0) score = -negaMax(-beta, -alpha, depth - 1, cutNode); //PV node - cutNode instead of true?
			else { //cut node: all heuristic allowed
				int r = 0; //late move reduction factor
				r += (moveSearched > 1 + 2 * (!cutNode));
				if(okToReduce(currMove) && !inCheck) r += min(5, (((int)(21.9 * log(ply))) * (15 + 7 * moveSearched) + 534) / 1024);
				r += (historyMoves[ply][getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
				score = -negaMax(-alpha - 1, -alpha, depth - r - 1, true);

				if (score > alpha) {

					score = -negaMax(-alpha - 1, -alpha, depth - 1, true);

					if ((score > alpha) && (score < beta)) score = -negaMax(-beta, -alpha, depth - 1, cutNode);

				}
			}

			//Undo move
			pos = save;
			++moveSearched;
			--ply;
			--repetitionIndex;

			// good move?
			if (score > alpha) {
				bestMove = currMove;
				hashFlag = hashEXACT;
				historyMoves[ply][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				//if(ply)historyMoves[ply-1][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				if(ply>1)historyMoves[ply-2][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				//if(ply<maxPly)historyMoves[ply+1][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				if(ply<maxPly-1)historyMoves[ply+2][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
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
				//TOO GOOD OF A MOVE?
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
inline int Game::negaMax(int alpha, int beta, int depth, bool pv, double nulled) {
	if ((nodes & 2047) == 0) {
		communicate();
		if (stopped)return 0;
	}
	if (ply && isRepetition()) return 0;
	int hashFlag = hashALPHA;
	int score;
	if (!pv && nulled > 0)nulled -= 0.5;


	//bool pvNode = (beta - alpha) > 1;

	if (!pv && ply && (score = readHashEntry(pos.hashKey, alpha, beta, depth)) != 100000)
		return score;

	if (ply > maxPly - 1) {
		return eval();
	}
	pvLen[ply] = ply;
	if (depth <= 0)return quiescence(alpha, beta);

	//remove for spEEd


	++nodes;
	unsigned long kingPos;
	bitScanForward(&kingPos, (pos.side ? pos.bitboards[k] : pos.bitboards[K]));
	int inCheck = pos.isSquareAttacked(kingPos, pos.side ^ 1);
	if (inCheck) {
		++depth;
		if (getCaptureFlag(pos.lastMove))++depth;
	}
	else {


		// evaluation pruning / static null move pruning
		int staticEval = eval();


		if (depth < 3 && !pv && abs(beta - 1) > -infinity + 100) {
			// define evaluation margin
			int evalMargin = 120 * depth;

			// evaluation margin substracted from static evaluation score fails high
			if (staticEval - evalMargin >= beta)
				// evaluation margin substracted from static evaluation score
				return staticEval - evalMargin;
		}

		//Null MP
		if (!pv && depth >= 3 && ply) {
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
			//R += depth > 3;
			//R += depth > 5;
			//R += depth > 7;
			//R = min(R, 5 - nulled);
			int sscore = -negaMax(-beta, -beta + 1, depth - 1 - R, false, nulled + R);
			--ply;
			--repetitionIndex;

			pos.side ^= 1;
			pos.enPassant = exPassant;
			pos.hashKey ^= sideKeys;
			pos.hashKey ^= enPassantKeys[exPassant];

			//if (stopped == true)return 0;
			if (sscore >= beta) {
				return sscore;
				depth -= R;
				if (depth <= 0)return quiescence(alpha, beta);
			}
		}

		// razoring
		if (!pv && depth <= 3)
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

	}





	// number of moves searched in a move list
	int moveSearched = 0;


	moves* moveList = new moves;
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
	//IID (?)
#define useIID true
#if useIID == true
	if (depth > 5) {
		if (moveList->m[0] == 0 && moveList->m[1] < 0xFFFFFFFF00000000) {
			if (abs(beta - alpha) > 1) moveList->m[0] = IID(moveList, depth / 4 + 2);
			for (int i = 0; i < moveList->count; ++i) if (onlyMove(moveList->m[i]) == moveList->m[0]) {
				moveList->m[i] = 0;
				break;
			}
		}
	}
#endif
	
	//OTTIMIZZAZIONE : PREV POS IS THE SAME THROUGHT THE WHOLE FOR CYCLE, MAYBE NOT SAVING IN MAKEMOVE?
	moveInt currMove;
	moveInt bestMove = 0;
	for (int i = 0; i < moveList->count; ++i) {
		currMove = onlyMove(moveList->m[i]);
		//if (currMove == entry->move)continue;
		++ply;
		++repetitionIndex;
		repetitionTable[repetitionIndex] = pos.hashKey;

		if (makeMove(currMove)) {
			int score = alpha + 1;

			if (moveSearched == 0) score = -negaMax(-beta, -alpha, depth - 1, pv, nulled);
			else {
				bool recapture = (getCaptureFlag(pos.lastMove) > 0) * (getCaptureFlag(currMove) > 0);
				//bool sac = (getCaptureFlag(currMove)>0) * (squareBB(getMoveTarget) & 
				if(recapture) score = -negaMax(-alpha - 1, -alpha, depth - 1, false, nulled);
				else if (okToReduce(currMove) && !inCheck) {
					//if (moveSearched < overReduct) {
						//bool badStory = (historyMoves[ply][getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
						//badStory += (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= 0;
						score = -negaMax(-alpha - 1, -alpha, depth - 1 - sqrt(log(2*ply*moveSearched* moveList->count)), false, nulled);
					//}
					//else {

					//	bool badStory = (historyMoves[ply][getMovePiece(currMove)][getMoveTarget(currMove)]) <= depth;
						//badStory += (historyMoves[getMovePiece(currMove)][getMoveTarget(currMove)]) <= 0;
					//	score = -negaMax(-alpha - 1, -alpha, (int)(depth * 0.667) - badStory - 1 - (moveSearched>>4), false, nulled);
						

					}

				else score = alpha + 1;

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

				
				//if (ply)historyMoves[ply - 1][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				if (ply > 1)historyMoves[ply - 2][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				//if (ply < maxPly)historyMoves[ply + 1][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				if (ply < maxPly - 1)historyMoves[ply + 2][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
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
				historyMoves[ply][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				bestMove = currMove;
				hashFlag = hashEXACT;
				
				

				//writeHashEntry(pos.hashKey, alpha, depth-1, currMove, hashALPHA);
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

*/




inline int Game::negaMax(int alpha, int beta, int depth, bool pv, unsigned char nullMoveCounter) {
	//Communication
	if ((nodes & 2047) == 0) {
		communicate();
		if (stopped)return 0;
	}

	//repetition
	if (ply && isRepetition()) return 0;
	int score;

	//null move pruning reset
	if (!pv && nullMoveCounter > 0) --nullMoveCounter;

	//tt probing
	if (!pv && ply && (score = readHashEntry(pos.hashKey, alpha, beta, depth)) != 100000)
		return score;

	//avoid ply overflow
	if (ply > maxPly - 1)
		return eval();

	//adjust pvLen
	pvLen[ply] = ply;

	//drop into qSearch if depth <= 0
	if (depth <= 0) return quiescence(alpha, beta);

	/////////// CHECK FOR CHECK ///////////
	++nodes;
	unsigned long kingPos;
	bitScanForward(&kingPos, pos.bitboards[K + pos.side*6]);
	bool inCheck = pos.isSquareAttacked(kingPos, pos.side ^ 1);
	if (inCheck) {
		++depth;
		// uncomment for some questionable recapture extension
		// if(getCaptureFlag(pos.lastMove)) ++ depth; 
	}
	else {

		if (!pv) {
			// let the pruning galore commence ;)
			// evaluation pruning and razoring on depth < 3
			if (depth < 3) {

				//EVALUATION PRUNING
				int staticEval = 0;
				if (abs(beta - 1) > -infinity + 100) {
					//evalMargin
					int evalMargin = 120 * depth;

					//evaluation margin is subtracted from static score, if it fails high, return the upperBound
					if (staticEval - evalMargin >= beta)
						return staticEval - evalMargin;
				}

				//RAZORING
				//add first bounus
				score = staticEval + 125;

				//let's search for fail low nodes
				if (score < beta) {
					// on depth 1
					if (depth == 1) {
						// get quiscence score
						int newScore = quiescence(alpha, beta);

						// return quiescence score if it's greater then static evaluation score
						return (newScore > score) ? newScore : score;
					}

					// add second bonus to static evaluation
					score += 175;

					// static evaluation indicates a fail-low node
					if (score < beta && depth <= 2) {
						// get quiscence score
						int newScore = quiescence(alpha, beta);

						// quiescence score indicates fail-low node
						if (newScore < beta)
							// return quiescence score if it's greater then static evaluation score
							return (newScore > score) ? newScore : score;
					}
				}
			}
			else {

				//Null move pruning
				if (ply && nullMoveCounter == 0) {

					//null move is technically a ply
					++ply;

					//so increase the null move counter (???)
					++repetitionIndex;
					repetitionTable[repetitionIndex] = pos.hashKey;

					//do the null move
					pos.side ^= 1; //switch side
					int exPassant = pos.enPassant; // save last enPassant
					pos.enPassant = no_square; // remove enPassant square
					pos.hashKey ^= sideKeys; // hash the other side
					pos.hashKey ^= enPassantKeys[exPassant]; // hashOut the en passant square

					int R = 2;
					int nullScore = -negaMax(-beta, -beta + 1, -depth - 1 - R, false, nullMoveCounter + 2*R);

					--ply;
					--repetitionIndex;

					//undo the null move
					pos.side ^= 1; //switch side
					pos.enPassant = exPassant; // reload last enPassant
					pos.hashKey ^= sideKeys; // hash the other side
					pos.hashKey ^= enPassantKeys[exPassant]; // hashIn the en passant square

					if (nullScore >= beta) {
						// test 1
						//historyMoves[ply - 1][getMoveSource(pos.lastMove)][getMoveTarget(pos.lastMove)] -= (historyMoves[ply - 1][getMoveSource(pos.lastMove)][getMoveTarget(pos.lastMove)] > 0);
						depth -= R;
					}
				}
			}
		}
	}
	// END PRUNING

	int moveSearched = 0;

	moves* moveList = new moves;
	generateMoves(moveList, inCheck);
	Position save = pos;
	
	tt* entry = getEntry(pos.hashKey);
	//fetch entry bMove
	for (int i = 1; i < moveList->count; ++i) {
		if (onlyMove(moveList->m[i]) == entry->move) {
			//if there's already a PV move
			if (moveList->m[1] > 0xFFFFFFFF00000000) {
				moveList->m[0] = moveList->m[1];
				moveList->m[1] = moveList->m[i];
			}
			//else
			else {
				moveList->m[0] = moveList->m[i];
			}
			//in any case, the fetched move double must be removed
			moveList->m[i] = 0;
			break;
		}
	}

#define useIID true
#if useIID
	if (abs(beta - alpha) > 1 && depth > 5) {
		//if not fetched move from tt & no pv move
		if (moveList->m[0] == 0 && moveList->m[1] < 0xFFFFFFFF00000000) {
			moveList->m[0] = IID(moveList, depth / 4 + 2);
			// moveList->m[0] = IID(moveList, depth / 4 + 2); //replace the if ...?
			// remove doubled IID move
			for (int i = 0; i < moveList->count; ++i) if (onlyMove(moveList->m[i]) == moveList->m[0]) {
				moveList->m[i] = 0;
				break;
			}
		}
	}
#endif



	moveInt currMove;
	moveInt bestMove = 0;
	int hashFlag = hashALPHA;
	//iterate through moves
	for (int i = 0; i < moveList->count; ++i) {
		//set current move
		currMove = onlyMove(moveList->m[i]);

		//increase ply and repetition, store hashKey in repetition
		++ply;
		++repetitionIndex;
		repetitionTable[repetitionIndex] = pos.hashKey;

		//TODO: DO A LEGAL MOVE GENERATOR, NOT A PSEUDO LEGAL + MOVE VERIFICATION
		if (makeMove(currMove)) {
			//if legal
			int score = alpha + 1; //so that at the end of search(es), we can perform a little trick ;)
			if (!moveSearched) score = -negaMax(-beta, -alpha, depth - 1, pv, nullMoveCounter); //firstMove MUST be searched on full alpha-beta
			else {
				// another recapture stuff ?
				// bool recapture = getCaptureFlag(pos.lastMove) > 0 && getCaptureFlag(currMove) > 0;
				// score = -negaMax(-alpha-1, -alpha, depth-1,false,nullMoveCounter);
				if (okToReduce(currMove) && !inCheck) {
					score = -negaMax(-alpha - 1, -alpha, depth - 1 - sqrt(log(2 * ply * moveSearched * moveList->count)), false, nullMoveCounter);
				}
				else score = alpha + 1;

				//now we check if score has never been touched with our magic trick from before
				if (score > alpha) {
					score = -negaMax(-alpha - 1, -alpha, depth - 1, false, nullMoveCounter);
					if ((score > alpha) && (score < beta)) score = -negaMax(-beta, -alpha, depth - 1, pv, nullMoveCounter);
				}
			}
			

			//restore position for next iteration
			++moveSearched;
			--ply;
			--repetitionIndex;
			pos = save;

			//now score MUST be set, so we can actually do stuff
			if (score > alpha) {

				//new candidate node
				alpha = score;

				if (pv) {				
					//write pvMove
					pvTable[ply][ply] = currMove;
					//copy variation
					for (int j = ply + 1; j < pvLen[ply + 1]; ++j) {
						pvTable[ply][j] = pvTable[ply + 1][j];
					}
					// adjust pvLen
					pvLen[ply] = pvLen[ply + 1];
				}
					

				if (score >= beta) {
					//on fail high
					if (!getCaptureFlag(currMove)) {
						//update killer move
						killerMoves[1][ply] = killerMoves[0][ply];
						killerMoves[0][ply] = currMove;
						//update counter move history
						counterMoves[getMoveSource(pos.lastMove)][getMoveTarget(pos.lastMove)] = bestMove;
					}
					//we can finish the search here
					delete moveList;
					//write on ttTable
					writeHashEntry(pos.hashKey, beta, depth, currMove, hashBETA);
					//return upper bound
					return beta;
				}

				//put history move [[ MAYBE do it after score >= beta check]]
				historyMoves[ply][getMovePiece(currMove)][getMoveTarget(currMove)] += depth * depth;
				bestMove = currMove;
				hashFlag = hashEXACT; //at least one move has been found
			}
		}
		else {
			//undo move if illegal
			--ply;
			--repetitionIndex;
			pos = save;
		}
	}

	//at the end of the search, delete moveList and write on hashTable, the return alpha
	delete moveList;

	if (!moveSearched) {
		if (inCheck) {
			//checkmate
			writeHashEntry(pos.hashKey, alpha, depth, bestMove, hashFlag);
			return -mateValue + ply;
		}
		else {
			//stalemate
			writeHashEntry(pos.hashKey, 0, depth, bestMove, hashFlag);
			return 0;
		}
	}

	writeHashEntry(pos.hashKey, alpha, depth, bestMove, hashFlag);
	return alpha;
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
	//memset(occupancies, 0ULL, sizeof(occupancies));

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
	/*for (int piece = P; piece <= K; piece++) {
		//populate occupancy
		occupancies[white] |= bitboards[piece];
	}
	//BLACK
	for (int piece = p; piece <= k; piece++) {
		occupancies[black] |= bitboards[piece];
	}

	occupancies[both] = occupancies[white] | occupancies[black];*/
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
	U64 occupancies[3];
	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[1] | occupancies[0];
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

#define queenPromotion  0x9ffffff
#define knightPromotion 0x5ffffff
#define rookPromotion   0x4ffffff
#define bishopPromotion 0x3ffffff

#define checkBonus 0xffff
#define pawnPushBonus 0xbfff

inline void Position::generateMoves(moves* moveList, bool isCheck) {

	U64 occupancies[3];
	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[1] | occupancies[0];
	//reset the moveList
	moveList->count = 0;
	U64 notWhite = ~(occupancies[white]);
	U64 notBlack = ~(occupancies[black]);
	U64 noneOccupancy = notWhite & notBlack;
	unsigned long otherKing;
	bitScanForward(&otherKing, bitboards[K + (6 * side)]);
	U64 bishopCheckers = getBishopAttacks(otherKing, occupancies[both]) ;
	U64 rookCheckers = getRookAttacks(otherKing, occupancies[both]) ;
	U64 knightCheckers = knightAttacks[otherKing];
	U64 pawnCheckers = pawnAttacks[1 - side][otherKing] ;
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
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 0, 0, 0, 0), pawnPushBonus + checkBonus * ((squareBB(targetSquare) & pawnCheckers)>0));
							//double pushes
							targetSquare -= 8;
							if (sourceSquare >= a2 && !testBit(occupancies[both], targetSquare)) {
								addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 0, 1, 0, 0), pawnPushBonus + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
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
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
						}
						//normal pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, Q, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + queenPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, N, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + knightPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, R, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + rookPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, P, B, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + bishopPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[white][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, P, 0, 1, 0, 1, 0), 0x1fffff + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
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
					addMove(moveList, encodeMove(sourceSquare, targetSquare, K, 0, 1, 0, 0, 0), isCheck << 6);
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + knightCaptures + checkBonus * ((squareBB(targetSquare) & knightCheckers) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, N, 0, 0, 0, 0, 0), 3 + checkBonus * ((squareBB(targetSquare) & knightCheckers) > 0));
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + bishopCaptures + checkBonus * ((squareBB(targetSquare) & bishopCheckers) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, B, 0, 0, 0, 0, 0), 3 + checkBonus * ((squareBB(targetSquare) & bishopCheckers) > 0));
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + rookCaptures + checkBonus * ((squareBB(targetSquare) & rookCheckers) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, R, 0, 0, 0, 0, 0), 2 + checkBonus * ((squareBB(targetSquare) & rookCheckers) > 0));
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 1, 0, 0, 0), whiteCaptureValueAt(targetSquare) + queenCaptures + checkBonus * ((squareBB(targetSquare) & (bishopCheckers|rookCheckers)) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, Q, 0, 0, 0, 0, 0), 1 + checkBonus * ((squareBB(targetSquare) & (bishopCheckers | rookCheckers)) > 0));
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
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 0, 0, 0, 0), pawnPushBonus + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							//double pushes
							targetSquare += 8;
							if (sourceSquare <= h7 && !testBit(occupancies[both], targetSquare)) {
								addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 0, 1, 0, 0), pawnPushBonus + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							}
						}
						//promotion pushes
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 0, 0, 0, 0), queenPromotion + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 0, 0, 0, 0), knightPromotion + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 0, 0, 0, 0), rookPromotion + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 0, 0, 0, 0), bishopPromotion + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							
						}
					}

					//pawn attacks
					U64 attacks = pawnAttacks[black][sourceSquare] & occupancies[white];
					while (attacks) {
						//grab next pawn attack
						bitScanForward(&targetSquare, attacks);
						//pawn capture
						if (targetSquare < a1) {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
						}
						//promotion pawn capture
						else {
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, q, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + queenPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, n, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + knightPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, r, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + rookPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
							addMove(moveList, encodeMove(sourceSquare, targetSquare, p, b, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + bishopPromotion + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
						}
						clearBit(attacks, targetSquare); //clears analyzed attack
					}

					//en croissant captures
					if (enPassant != no_square) {
						//if available, check with capture mask
						if (pawnAttacks[black][sourceSquare] & squareBB(enPassant)) {
							//add croissant
							addMove(moveList, encodeMove(sourceSquare, enPassant, p, 0, 1, 0, 1, 0), 0x1fffff + pawnCaptures + checkBonus * ((squareBB(targetSquare) & pawnCheckers) > 0));
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
					addMove(moveList, encodeMove(sourceSquare, targetSquare, k, 0, 1, 0, 0, 0), isCheck << 6);
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + knightCaptures + checkBonus * ((squareBB(targetSquare) & knightCheckers) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, n, 0, 0, 0, 0, 0), 3 + checkBonus * ((squareBB(targetSquare) & knightCheckers) > 0));
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + bishopCaptures + checkBonus * ((squareBB(targetSquare) & bishopCheckers) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, b, 0, 0, 0, 0, 0), 3 + checkBonus * ((squareBB(targetSquare) & bishopCheckers) > 0));
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + rookCaptures + checkBonus * ((squareBB(targetSquare) & rookCheckers) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, r, 0, 0, 0, 0, 0), 2 + checkBonus * ((squareBB(targetSquare) & rookCheckers) > 0));
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
						addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 1, 0, 0, 0), blackCaptureValueAt(targetSquare) + queenCaptures + checkBonus * ((squareBB(targetSquare) & (rookCheckers | bishopCheckers)) > 0));
						clearBit(attacks, targetSquare);
					}
					while (quiets) {
						bitScanForward(&targetSquare, quiets);
						addMove(moveList, encodeMove(sourceSquare, targetSquare, q, 0, 0, 0, 0, 0), 1 + checkBonus * ((squareBB(targetSquare) & (rookCheckers | bishopCheckers)) > 0));
						clearBit(quiets, targetSquare);
					}
					clearBit(bitboard, sourceSquare);
				}
			}
		}
	}

	std::sort(std::begin(moveList->m), std::begin(moveList->m)+moveList->count, std::greater<U64>());
}

moveInt Game::IID(moves* moveList, int depth) {
	//first, we generate a move list
	
	unsigned char bestMove = 0;
	int score = -infinity;
	Position save = pos;
	for (int i = 0; i < moveList->count; ++i) {
		++ply;
		++repetitionIndex;
		repetitionTable[repetitionIndex] = pos.hashKey;
		if (makeMove(moveList->m[i])) {
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
	return moveList->m[bestMove];
}

inline void Position::generateCaptures(moves* moveList) {
	U64 occupancies[3];
	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[1] | occupancies[0];
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
		move |= (0xffffffffff000000);
	}
	else if (killerMoves[0][ply] == move) {
		move |= (0xffffff000000);
	}
	else if (killerMoves[1][ply] == move) {
		move |= (0xefffff000000);
	}
	else if (ply < 62 && killerMoves[0][ply+2] == move) {
		move |= (0xcfffff000000);
	}
	else if (ply < 62 && killerMoves[1][ply+2] == move) {
		move |= (0xbfffff000000);
	}
	else if (ply > 2 && killerMoves[0][ply - 2] == move) {
		move |= (0xcfffff000000);
	}
	else if (ply > 2 && killerMoves[1][ply - 2] == move) {
		move |= (0xbfffff000000);
	}
	else if (killerMoves[0][ply] == move) {
		move |= (0xffffff000000);
	}
	else if (killerMoves[1][ply] == move) {
		move |= (0xefffff000000);
	}
	else if (move == counterMoves[getMoveSource(lastMove)][getMoveTarget(lastMove)]) {
		move |= (0xdfffff000000);
	}
	else {
#define centerBonus 10 //10 > 7 > 2 > 16
		if (ply >= 2) {
			bonus += historyMoves[ply - 2][getMovePiece(move)][getMoveTarget(move)] >> 2;
			if (ply >= 4) bonus += historyMoves[ply - 4][getMovePiece(move)][getMoveTarget(move)] >> 4;
		}
		bonus += historyMoves[ply][getMovePiece(move)][getMoveTarget(move)];
		if (ply <= 61) {
			bonus += historyMoves[ply + 2][getMovePiece(move)][getMoveTarget(move)] >> 2;
			if (ply <= 59) bonus += historyMoves[ply + 4][getMovePiece(move)][getMoveTarget(move)] >> 4;
		}
		bonus += centerBonusTable10[getMoveTarget(move)];
		
		move |= ((U64)bonus << 24);
	}

	moveList->m[moveList->count] = move;
	++moveList->count;


}