#include "pestoEval.h"
#include <math.h>

/* piece/sq tables */
/* values from Rofchade: http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19 */

int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -30, -30,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  -30,  -30,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

int* mg_pesto_table[6] =
{
    mg_pawn_table,
    mg_knight_table,
    mg_bishop_table,
    mg_rook_table,
    mg_queen_table,
    mg_king_table
};

int* eg_pesto_table[6] =
{
    eg_pawn_table,
    eg_knight_table,
    eg_bishop_table,
    eg_rook_table,
    eg_queen_table,
    eg_king_table
};

int gamephaseInc[12] = { 0,0,1,1,1,1,2,2,4,4,0,0 };
int mg_table[12][64];
int eg_table[12][64];
int manhattanDistance[64][64] = { 0 };

void init_tables() {

    for (int x1 = 0; x1 < 8; x1++) {
        for (int y1 = 0; y1 < 8; y1++) {
            for (int x2 = 0; x2 < 8; x2++) {
                for (int y2 = 0; y2 < 8; y2++) {
                    manhattanDistance[x1 + y1 * 8][x2 + y2 * 8] = std::min(abs(x1-x2),abs(y1-y2));
                }
            }
        }
    }

    int pc, p, sq;
    for (p = PAWN, pc = WHITE_PAWN; p <= KING; pc += 2, p++) {
        for (sq = 0; sq < 64; sq++) {
            mg_table[pc][sq] = mg_value[p] + mg_pesto_table[p][sq];
            eg_table[pc][sq] = eg_value[p] + eg_pesto_table[p][sq];
            mg_table[pc + 1][sq] = mg_value[p] + mg_pesto_table[p][FLIP(sq)];
            eg_table[pc + 1][sq] = eg_value[p] + eg_pesto_table[p][FLIP(sq)];
        }
    }
}


#define pawnChainBonus 5
#define kingFileBonus 5
#define centralShield 15
#define sideShield 3
#define bishopColorWeakness 5
#define openFileRook 10

#define advancedPawnEval true
#define advancedKnightEval true
#define advancedBishopEval true
#define advancedRookEval true
#define advancedQueenEval true

#define mobilityEval true

//const int mg_value[6] = { 82, 337, 365, 477, 1025,  0 };
int pestoEval(Position* pos) {
    U64 occupancies[3];
    occupancies[0] = pos->bitboards[0] | pos->bitboards[1] | pos->bitboards[2] | pos->bitboards[3] | pos->bitboards[4] | pos->bitboards[5];
    occupancies[1] = pos->bitboards[6] | pos->bitboards[7] | pos->bitboards[8] | pos->bitboards[9] | pos->bitboards[10] | pos->bitboards[11];
    occupancies[2] = occupancies[1] | occupancies[0];
    U64 freeOccupancy = ~occupancies[2];
    //DRAW BY INSUFFICIENT MATERIAL
    double draw = 0;
    for (int i = 0 + (6 * pos->side); i < 6 + (6 * pos->side); ++i) {
        draw += popcount(pos->bitboards[i])*drawVals[i%6];
    }


    int score = 0; //for end games
    bool isEndGame = false;
    char endGame = 0;

    if (popcount(occupancies[black]) == 1) {
        isEndGame = true;

        endGame = 1; //White winning
    }

    if (popcount(occupancies[white]) == 1) {
        isEndGame = true;

        if (endGame)return 0; //K v K
         endGame = 2; //black winning
    }

    if (isEndGame) {
        if (endGame == 0)return 0;



        if (endGame == 1) {
            unsigned long losingKing;
            bitScanForward(&losingKing, pos->bitboards[11]);
            score += endKingTable[losingKing];
            unsigned long winningKing;
            bitScanForward(&winningKing, pos->bitboards[5]);
            score += (manhattanDistance[winningKing][losingKing] == 2) * 100;
        }
        else {
            unsigned long losingKing;
            bitScanForward(&losingKing, pos->bitboards[5]);
            score -= endKingTable[losingKing];
            unsigned long winningKing;
            bitScanForward(&winningKing, pos->bitboards[11]);
            score += (manhattanDistance[winningKing][losingKing] == 2) * 100;
        }
    }
    
    //if K v KP
    
    int mg[2];
    int eg[2];
    int gamePhase = 0;
    

    mg[WHITE] = 0;
    mg[BLACK] = 0;
    eg[WHITE] = 0;
    eg[BLACK] = 0;

    /*
    
    WhitePiecesOnWhiteSquare
    WhitePiecesOnBlackSquare
    
    BlackPiecesOnWhiteSquare
    BlackPiecesOnBlackSquare

    whiteWeaknessOnWhiteSquares = (BlackPiecesOnBlack/blackPieces);
    if (whiteSquaredBishop){
    }
    else{
        score -= (whiteWeaknessOnWhiteSquares + 1)*(whiteWeaknessOnWhiteSquares + 1);
    }
    */


    /* evaluate each piece */
    U64* bitboards = pos->bitboards;
    unsigned long wk;
    unsigned long bk;
    bitScanReverse(&wk, bitboards[5]);
    bitScanReverse(&bk, bitboards[11]);

    int tropismToWhiteKing = 0;
    int tropismToBlackKing = 0;
    //int colorWeakness = 0;
    U64 bitboard = bitboards[0];
    //pawn count
    int whitePawnCount = popcount(pos->bitboards[0]);
    int blackPawnCount = popcount(pos->bitboards[6]);
    //mat
    int whiteMat = 0;
    
   


    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        //score += pawnChainBonus * ((int)popcount(pawnAttacks[0][sq] & bitboards[0])) + 3 * ((int)popcount(pawnAttacks[0][sq] & (bitboards[7] | bitboards[8]))); //pawn chains 
        mg[0] += mg_table[0][sq];
        eg[0] += eg_table[0][sq];
        whiteMat += 82;
        gamePhase += gamephaseInc[0];
#if advancedPawnEval == true
        if ((squaresAhead[white][sq & 7] & occupancies[black]) == 0) score += (8 - (sq >> 3)) <<3; //passed pawns
        if ((squaresAhead[white][sq & 7] & pos->bitboards[0])) score -= 45; //doubled pawns
        if ((sq == e2 || sq == d2) && (occupancies[both] & squareBB(sq-8)))score -= 35; //blocked d2/e2 pawn
        if (popcount(isolated[sq & 7] & pos->bitboards[0]) <= 1) score -= 10 * (sq >> 3); //isolated backward
        score -= popcount(pawnAttacks[white][sq - 8] & bitboards[p])*4; //indirectly blocked pawns
        score -= (testBit(occupancies[black], sq) * (sq >> 3) * 20); //blocked pawns
#endif

    }

    //mat
    int blackMat = 0;
    bitboard = bitboards[6];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        //score -= pawnChainBonus * ((int)popcount(pawnAttacks[1][sq] & bitboards[6])) + 3 * ((int)popcount(pawnAttacks[1][sq] & (bitboards[1] | bitboards[2]))); //pawn chains 
        mg[1] += mg_table[1][sq];
        eg[1] += eg_table[1][sq];
        blackMat += 82;
        gamePhase += gamephaseInc[1];
#if advancedPawnEval == true
        if ((squaresAhead[black][sq & 7] & occupancies[white]) == 0) score -= (sq >> 3) << 3; //passed pawns
        if ((squaresAhead[white][sq & 7] & pos->bitboards[0])) score += 45; //doubled pawns
        if ((sq == e7 || sq == d7) && (occupancies[both] & squareBB(sq+8)))score += 35; //blocked d7/e7 pawn
        if (popcount(isolated[sq & 7] & pos->bitboards[6]) <= 1) score += 10 * (8-(sq >> 3)); //isolated backward
        score += popcount(pawnAttacks[black][sq + 8] & bitboards[P]) * 4; //indirectly blocked pawns
        score += (testBit(occupancies[white], sq) * (8 - (sq >> 3)) * 10); //blocked pawns
#endif
    }

    bitboard = bitboards[1];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[0] += mg_table[2][sq];
        eg[0] += eg_table[2][sq];
        gamePhase += gamephaseInc[2];
        whiteMat += 337;
        tropismToBlackKing += nkdist[sq][bk];
#if advancedKnightEval == true
        score -= (8 - whitePawnCount)*4; //best on closed positions
        score += 8 * ((pawnAttacks[black][sq] & pos->bitboards[0]) > 0); //defended by pawn
#endif


#if mobilityEval == true
        score += 6 * popcount(knightAttacks[sq] & freeOccupancy);
        score += 2 * popcount(knightAttacks[sq] & occupancies[black]);
#endif
    }

    bitboard = bitboards[7];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[1] += mg_table[3][sq];
        eg[1] += eg_table[3][sq];
        gamePhase += gamephaseInc[3];
        blackMat += 337;
        tropismToWhiteKing += nkdist[sq][wk];
#if advancedKnightEval == true
        score += (8 - blackPawnCount)*4; //best on closed positions
        score -= 8 * ((pawnAttacks[white][sq] & pos->bitboards[6]) > 0); //defended by pawn
#endif
#if mobilityEval == true
        score -= 6 * popcount(knightAttacks[sq] & freeOccupancy);
        score -= 2 * popcount(knightAttacks[sq] & occupancies[white]);
#endif
    }

    bitboard = bitboards[2];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[0] += mg_table[4][sq];
        eg[0] += eg_table[4][sq];
        gamePhase += gamephaseInc[4];
        whiteMat += 365;
        tropismToBlackKing += kbdist[sq][bk];
#if advancedBishopEval == true
        if ((pos->bitboards[5] & squareBB(sq + 8)) && (sq == g2 || sq == b2))score += 14; //fianchetto bonus
        
#endif
#if mobilityEval == true
        U64 attacks = getBishopAttacks(sq, occupancies[both]);
        score += 8 * popcount(attacks & freeOccupancy);
        score += 3 * popcount(attacks & occupancies[black]);
#endif
    }
#if advancedBishopEval == true
    if (popcount(pos->bitboards[2]) == 2)score += ((8 - blackPawnCount) * (8 - blackPawnCount)); //bishop pair
#endif

    bitboard = bitboards[8];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[1] += mg_table[5][sq];
        eg[1] += eg_table[5][sq];
        gamePhase += gamephaseInc[5];
        blackMat += 365;
        tropismToWhiteKing += kbdist[sq][wk];
#if advancedBishopEval == true
        if ((pos->bitboards[11] & squareBB(sq - 8)) && (sq == g7 || sq == b7))score -= 14; //fianchetto bonus
        
#endif
#if mobilityEval == true
        U64 attacks = getBishopAttacks(sq, occupancies[both]);
        score -= 8 * popcount(attacks & freeOccupancy);
        score -= 3 * popcount(attacks & occupancies[white]);
#endif
    }
#if advancedBishopEval == true
    if (popcount(pos->bitboards[8]) == 2)score -= ((8 - blackPawnCount) * (8 - blackPawnCount)); //bishop pair
#endif

    bitboard = bitboards[3];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[0] += mg_table[6][sq];
        eg[0] += eg_table[6][sq];
        whiteMat += 477;
        //score += ((bitboards[11] & files[sq % 8]) > 0) * kingFileBonus + ((bitboards[6] & files[sq % 8]) == 0) * openFileRook;
        gamePhase += gamephaseInc[6];
        tropismToBlackKing += rkdist[sq][bk];
#if advancedRookEval == true
        score += popcount(files[sq & 7] & (pos->bitboards[10] | pos->bitboards[11])) * 10;
        score += (popcount(ranks[sq >> 3] & pos->bitboards[3]) >= 2) * 15;
        score += (!(squaresAhead[white][sq & 7] & pos->bitboards[6])) * 10; //semi-open file bonus
        score += (!(squaresAhead[white][sq & 7] & occupancies[black])) * 15; //open file bonus
#endif
#if mobilityEval == true
        U64 attacks = getRookAttacks(sq, occupancies[both]);
        score += 8 * popcount(attacks & freeOccupancy);
        score += 4 * popcount(attacks & occupancies[black]);
#endif
    }

    bitboard = bitboards[9];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[1] += mg_table[7][sq];
        eg[1] += eg_table[7][sq];
        blackMat += 477;
        //score -=  ((bitboards[5]  & files[sq % 8]) > 0) * kingFileBonus + ((bitboards[0] & files[sq % 8]) == 0) * openFileRook;
        gamePhase += gamephaseInc[7];
        tropismToWhiteKing += rkdist[sq][wk];
#if advancedRookEval == true
        score -= popcount(files[sq & 7] & (pos->bitboards[4] | pos->bitboards[5])) * 10;
        score -= (popcount(ranks[sq >> 3] & pos->bitboards[9]) >= 2) * 15;
        score -= (!(squaresAhead[black][sq & 7] & pos->bitboards[0])) * 10; //semi-open file bonus
        score -= (!(squaresAhead[black][sq & 7] & occupancies[white])) * 15; //open file bonus
#endif
#if mobilityEval == true
        U64 attacks = getRookAttacks(sq, occupancies[both]);
        score -= 8 * popcount(attacks & freeOccupancy);
        score -= 4 * popcount(attacks & occupancies[white]);
#endif
    }

    bitboard = bitboards[4];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[0] += mg_table[8][sq];
        eg[0] += eg_table[8][sq];
        gamePhase += gamephaseInc[8];
        whiteMat += 1025;
        tropismToBlackKing += qkdist[sq][bk];
#if advancedQueenEval == true
        //bad queen mobility malus
        int qMob = 8 - popcount(occupancies[both] & kingAttacks[sq]);
        score -= (qMob * qMob) >> 2;
#endif
#if mobilityEval == true
        U64 attacks = getQueenAttacks(sq, occupancies[both]);
        score += 5 * popcount(attacks & freeOccupancy);
        score += 3 * popcount(attacks & occupancies[black]);
#endif
    }

    bitboard = bitboards[10];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[1] += mg_table[9][sq];
        eg[1] += eg_table[9][sq];
        gamePhase += gamephaseInc[9];
        blackMat += 1025;
        tropismToWhiteKing += kbdist[sq][wk];
#if advancedQueenEval == true
        //bad queen mobility malus
        int qMob = 8 - popcount(occupancies[both] & kingAttacks[sq]);
        score += (qMob * qMob) >> 2;
#endif
#if mobilityEval == true
        U64 attacks = getQueenAttacks(sq, occupancies[both]);
        score -= 5 * popcount(attacks & freeOccupancy);
        score -= 3 * popcount(attacks & occupancies[white]);
#endif
    }

    bitboard = bitboards[5];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[0] += mg_table[10][sq];
        eg[0] += eg_table[10][sq];
        gamePhase += gamephaseInc[10];
    }

    bitboard = bitboards[11];
    while (bitboard) {
        unsigned long sq;
        bitScanReverse(&sq, bitboard);
        clearBit(bitboard, sq);
        mg[1] += mg_table[11][sq];
        eg[1] += eg_table[11][sq];
        gamePhase += gamephaseInc[11];
    }

    /* tapered eval */
    //TODO: make a pawnShield mask
    int whiteKingShield = ((wk>>3) >0? popcount(bitboards[0] & kingAttacks[wk] & ranks[(wk>>3) - 1]) - 3 : -3);
    int blackKingShield = ((wk >> 3) <7? popcount(bitboards[6] & kingAttacks[bk] & ranks[(bk >> 3) + 1]) - 3: -3);
#define INITIAL_PIECE_MATERIAL 4039
    int whiteKingSafety = 2 * ((whiteKingShield - tropismToWhiteKing) * whiteMat) / INITIAL_PIECE_MATERIAL;
    int blackKingSafety = 2 * ((blackKingShield - tropismToBlackKing) * blackMat) / INITIAL_PIECE_MATERIAL;

    int side2move = pos->side;
    int mgScore = mg[side2move] - mg[OTHER(side2move)];
    mgScore += whiteKingSafety - blackKingSafety;
    int egScore = eg[side2move] - eg[OTHER(side2move)];
    int mgPhase = gamePhase;
    if (mgPhase > 24) mgPhase = 24; /* in case of early promotion */
    int egPhase = 24 - mgPhase;


    /*
     score -= (int)(popcount(bitboards[0] & FILE_A) - 1) * 20;
     score -= (int)(popcount(bitboards[0] & FILE_B) - 1) * 15;
     score -= (int)(popcount(bitboards[0] & FILE_C) - 1) * 15;
     score -= (int)(popcount(bitboards[0] & FILE_D) - 1) * 25;
     score -= (int)(popcount(bitboards[0] & FILE_E) - 1) * 25;
     score -= (int)(popcount(bitboards[0] & FILE_F) - 1) * 15;
     score -= (int)(popcount(bitboards[0] & FILE_G) - 1) * 15;
     score -= (int)(popcount(bitboards[0] & FILE_H) - 1) * 20;

     score += (int)(popcount(bitboards[6] & FILE_A) - 1) * 20;
     score += (int)(popcount(bitboards[6] & FILE_B) - 1) * 15;
     score += (int)(popcount(bitboards[6] & FILE_C) - 1) * 15;
     score += (int)(popcount(bitboards[6] & FILE_D) - 1) * 25;
     score += (int)(popcount(bitboards[6] & FILE_E) - 1) * 25;
     score += (int)(popcount(bitboards[6] & FILE_F) - 1) * 15;
     score += (int)(popcount(bitboards[6] & FILE_G) - 1) * 15;
     score += (int)(popcount(bitboards[6] & FILE_H) - 1) * 20;

     for (int file = 0; file < 8; file++) {
         if (bitboards[0] & files[file]) {
             if (bitboards[6] & ((files[file]) == 0)) {
                 score += 10;
                 if (pos->occupancies[black] & ((files[file]) == 0)) {
                     score += 20;
                 }
             }
         }
         else if (bitboards[6] & files[file]) {
             if (bitboards[0] & ((files[file]) == 0)) {
                 score -= 10;
                 if (pos->occupancies[white] & ((files[file]) == 0)) {
                     score -= 20;
                 }
             }
         }
     }

     score += 10 * (popcount(bitboards[2]) >= 2);
     score -= 10 * (popcount(bitboards[8]) >= 2);
     */



    

    score += 28 * (1 - 2 * pos->side);
    return (mgScore * mgPhase + egScore * egPhase) / 24 + score;
}