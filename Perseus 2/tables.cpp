#include "tables.h"
#include "pestoEval.h"
#include <intrin.h>

#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(__popcnt64)
U64 pawnAttacks[2][64];
U64 knightAttacks[64] = { 0x20400				,0x50800			,0xa1100			,0x142200			,0x284400			,0x508800			,0xa01000			,0x402000		,
						0x2040004			,0x5080008			,0xa110011			,0x14220022			,0x28440044			,0x50880088			,0xa0100010			,0x40200020			,
						0x204000402			,0x508000805		,0xa1100110a		,0x1422002214		,0x2844004428		,0x5088008850		,0xa0100010a0		,0x4020002040		,
						0x20400040200		,0x50800080500		,0xa1100110a00		,0x142200221400		,0x284400442800		,0x508800885000		,0xa0100010a000		,0x402000204000		,
						0x2040004020000		,0x5080008050000	,0xa1100110a0000	,0x14220022140000	,0x28440044280000	,0x50880088500000	,0xa0100010a00000	,0x40200020400000	,
						0x204000402000000	,0x508000805000000	,0xa1100110a000000	,0x1422002214000000 ,0x2844004428000000 ,0x5088008850000000 ,0xa0100010a0000000 ,0x4020002040000000 ,
						0x400040200000000	,0x800080500000000	,0x1100110a00000000 ,0x2200221400000000 ,0x4400442800000000 ,0x8800885000000000 ,0x100010a000000000 ,0x2000204000000000 ,
						0x4020000000000		,0x8050000000000	,0x110a0000000000	,0x22140000000000	,0x44280000000000	,0x88500000000000	,0x10a00000000000	,0x20400000000000 };
U64 kingAttacks[64] = { 0x302				,0x705				,0xe0a				,0x1c14				,0x3828				,0x7050				,0xe0a0				,0xc040					,
						0x30203				,0x70507			,0xe0a0e			,0x1c141c			,0x382838			,0x705070			,0xe0a0e0			,0xc040c0				,
						0x3020300			,0x7050700			,0xe0a0e00			,0x1c141c00			,0x38283800			,0x70507000			,0xe0a0e000			,0xc040c000				,
						0x302030000			,0x705070000		,0xe0a0e0000		,0x1c141c0000		,0x3828380000		,0x7050700000		,0xe0a0e00000		,0xc040c00000			,
						0x30203000000		,0x70507000000		,0xe0a0e000000		,0x1c141c000000		,0x382838000000		,0x705070000000		,0xe0a0e0000000		,0xc040c0000000			,
						0x3020300000000		,0x7050700000000	,0xe0a0e00000000	,0x1c141c00000000	,0x38283800000000	,0x70507000000000	,0xe0a0e000000000	,0xc040c000000000		,
						0x302030000000000	,0x705070000000000	,0xe0a0e0000000000	,0x1c141c0000000000 ,0x3828380000000000 ,0x7050700000000000 ,0xe0a0e00000000000 ,0xc040c00000000000		,
						0x203000000000000	,0x507000000000000	,0xa0e000000000000	,0x141c000000000000	,0x2838000000000000	,0x5070000000000000 ,0xa0e0000000000000 ,0x40c0000000000000 };

U64 rookMagicNumbers[64] = { 0x8a80104000800020ULL ,
							 0x140002000100040ULL ,
							 0x2801880a0017001ULL ,
							 0x100081001000420ULL ,
							 0x200020010080420ULL ,
							 0x3001c0002010008ULL ,
							 0x8480008002000100ULL ,
							 0x2080088004402900ULL ,
							 0x800098204000ULL ,
							 0x2024401000200040ULL ,
							 0x100802000801000ULL ,
							 0x120800800801000ULL ,
							 0x208808088000400ULL ,
							 0x2802200800400ULL ,
							 0x2200800100020080ULL ,
							 0x801000060821100ULL ,
							 0x80044006422000ULL ,
							 0x100808020004000ULL ,
							 0x12108a0010204200ULL ,
							 0x140848010000802ULL ,
							 0x481828014002800ULL ,
							 0x8094004002004100ULL ,
							 0x4010040010010802ULL ,
							 0x20008806104ULL ,
							 0x100400080208000ULL ,
							 0x2040002120081000ULL ,
							 0x21200680100081ULL ,
							 0x20100080080080ULL ,
							 0x2000a00200410ULL ,
							 0x20080800400ULL ,
							 0x80088400100102ULL ,
							 0x80004600042881ULL ,
							 0x4040008040800020ULL ,
							 0x440003000200801ULL ,
							 0x4200011004500ULL ,
							 0x188020010100100ULL ,
							 0x14800401802800ULL ,
							 0x2080040080800200ULL ,
							 0x124080204001001ULL ,
							 0x200046502000484ULL ,
							 0x480400080088020ULL ,
							 0x1000422010034000ULL ,
							 0x30200100110040ULL ,
							 0x100021010009ULL ,
							 0x2002080100110004ULL ,
							 0x202008004008002ULL ,
							 0x20020004010100ULL ,
							 0x2048440040820001ULL ,
							 0x101002200408200ULL ,
							 0x40802000401080ULL ,
							 0x4008142004410100ULL ,
							 0x2060820c0120200ULL ,
							 0x1001004080100ULL ,
							 0x20c020080040080ULL ,
							 0x2935610830022400ULL ,
							 0x44440041009200ULL ,
							 0x280001040802101ULL ,
							 0x2100190040002085ULL ,
							 0x80c0084100102001ULL ,
							 0x4024081001000421ULL ,
							 0x20030a0244872ULL ,
							 0x12001008414402ULL ,
							 0x2006104900a0804ULL ,
							 0x1004081002402ULL };
U64 bishopMagicNumbers[64] = {	0x40040844404084ULL ,
								0x2004208a004208ULL ,
								0x10190041080202ULL ,
								0x108060845042010ULL ,
								0x581104180800210ULL ,
								0x2112080446200010ULL ,
								0x1080820820060210ULL ,
								0x3c0808410220200ULL ,
								0x4050404440404ULL ,
								0x21001420088ULL ,
								0x24d0080801082102ULL ,
								0x1020a0a020400ULL ,
								0x40308200402ULL ,
								0x4011002100800ULL ,
								0x401484104104005ULL ,
								0x801010402020200ULL ,
								0x400210c3880100ULL ,
								0x404022024108200ULL ,
								0x810018200204102ULL ,
								0x4002801a02003ULL ,
								0x85040820080400ULL ,
								0x810102c808880400ULL ,
								0xe900410884800ULL ,
								0x8002020480840102ULL ,
								0x220200865090201ULL ,
								0x2010100a02021202ULL ,
								0x152048408022401ULL ,
								0x20080002081110ULL ,
								 0x4001001021004000ULL ,
								0x800040400a011002ULL ,
								0xe4004081011002ULL ,
								0x1c004001012080ULL ,
								0x8004200962a00220ULL ,
								0x8422100208500202ULL ,
								0x2000402200300c08ULL ,
								0x8646020080080080ULL ,
								0x80020a0200100808ULL ,
								0x2010004880111000ULL ,
								0x623000a080011400ULL ,
								0x42008c0340209202ULL ,
								0x209188240001000ULL ,
								0x400408a884001800ULL ,
								0x110400a6080400ULL ,
								0x1840060a44020800ULL ,
								0x90080104000041ULL ,
								0x201011000808101ULL ,
								0x1a2208080504f080ULL ,
								0x8012020600211212ULL ,
								0x500861011240000ULL ,
								0x180806108200800ULL ,
								0x4000020e01040044ULL ,
								0x300000261044000aULL ,
								0x802241102020002ULL ,
								0x20906061210001ULL ,
								0x5a84841004010310ULL ,
								0x4010801011c04ULL ,
								0xa010109502200ULL ,
								0x4a02012000ULL ,
								0x500201010098b028ULL ,
								0x8040002811040900ULL ,
								0x28000010020204ULL ,
								0x6000020202d0240ULL ,
								0x8918844842082200ULL ,
								0x4010011029020020ULL };

U64 bishopMasks[64];
U64 rookMasks[64];
U64 bishopAttacks[64][512];
U64 rookAttacks[64][4096];
int distBonus[64][64];
int qkdist[64][64];
int rkdist[64][64];
int nkdist[64][64];
int bkdist[64][64];
int kbdist[64][64];

//relevancy occupancy bit count for each square for bishop
const int bishopRelevantBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

//relevancy occupancy bit count for each square for rook
const int rookRelevantBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

unsigned int state = 1804289383; //seed for magic number generation

//generate 32-bit pseudo random numbers

unsigned int getRandomNumber32() {
	//get current state
	unsigned int number = state;
	//XOR shift algorithm
	number ^= number << 13;
	number ^= number >> 17;
	number ^= number << 5;
	//update random number state;
	state = number;
	//return random number;
	return number;
}

U64 getRandomNumber64() {
	//define 4 random numbers
	U64 n1 = (U64)(getRandomNumber32()) & 0xFFFF; //Slicing first 16 most significant bits
	U64 n2 = (U64)(getRandomNumber32()) & 0xFFFF; //idem
	U64 n3 = (U64)(getRandomNumber32()) & 0xFFFF; //idem
	U64 n4 = (U64)(getRandomNumber32()) & 0xFFFF; //idem
	//return random number
	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48); //we sliced 16 bits from each
}

U64 findMagicNumber(int square, int relevantBits, int bishop){

	//initialize occupancies
	U64 occupancies[4096]; //max size for rooks

	//initialize attack tables
	U64 attacks[4096];

	//initialize used attacks
	U64 usedAttacks[4096];

	//initialize attack mask for curr piece
	U64 attackMask = bishop ? maskBishopAttacks(square) : maskRookAttacks(square);

	// initialize occupancy indexs
	int occupancyIndexs = squareBB(relevantBits);

	//loop over occupancy indexs
	for (int index = 0; index < occupancyIndexs; index++) {
		//init occupancies
		occupancies[index] = setOccupancy(index, relevantBits, attackMask);
		//init attacks
		attacks[index] = bishop ? bishopAttacksOnTheFly(square, occupancies[index]) : rookAttacksOnTheFly(square, occupancies[index]);
	}

	for (int randCount = 0; randCount < 10000000; randCount++) {
		// generate random number candidate
		U64 magicNumber = generateMagicNumber();

		// skip inappropriate numbers (WHY????? who cares)
		if (popcount((attackMask*magicNumber)&0xFF00000000000000) < 6) continue;

		//init used attacks
		memset(usedAttacks, 0ULL, sizeof(usedAttacks));

		//initialize index & fail flag
		int index, fail;

		//test magic indexs loop
		for (index = 0,fail=0; !fail && index < occupancyIndexs ; index++) {
			//init magic indexs
			int magicIndex = (int)((occupancies[index] * magicNumber) >> (64 - relevantBits)); //Here's the "MAGIC"
			//if magic index works initialize used attacks
			if (usedAttacks[magicIndex] == 0ULL) usedAttacks[magicIndex] = attacks[index];
			//otherwise index doesn't work
			else if (usedAttacks[magicIndex] != attacks[index])fail = 1;
		}

		//if magic number works, return it
		if (!fail) return magicNumber;
	}
	std::cout << "no magic number (???)" << "\n";
	return 0ULL;

}

U64 generateMagicNumber() {
	return getRandomNumber64() & getRandomNumber64() & getRandomNumber64();
}

void initMagicNumbers() {
	for (int square = 0; square < 64; square++) {
		//init rooks
		rookMagicNumbers[square] = findMagicNumber(square, rookRelevantBits[square], 0); //rook flag
		printf(" 0x%llxULL ,\n", rookMagicNumbers[square]); 
	}
	printf("\n\n");
	for (int square = 0; square < 64; square++) {
		bishopMagicNumbers[square] = findMagicNumber(square, bishopRelevantBits[square], 1); //bishop flag
		printf(" 0x%llxULL ,\n", bishopMagicNumbers[square]); 
	}
}

void initializePawnAttacks() {
	for (int side = 0; side < 2; side++) for (int square = 0; square < 64; square++)pawnAttacks[side][square] = maskPawnAttacks(square, side);
};
void initializeLeaperAttacks() {
	initializePawnAttacks();
};

void initAll() {
	initializePawnAttacks();
	initSlidersAttacks(1);
	initSlidersAttacks(0);
	initHashKeys();
	initTropism();
	init_tables();

	initBBTables();
	//initMagicNumbers();
}

U64 squaresAhead[2][64];
void initBBTables() {
	//passed pawn tables
	for (int square = 0; square < 64; square++) {
		//iterate through squares
		//file & (ffffffffffffffff<<square)
		squaresAhead[0][square] = files[square & 7] & (0xfffffffffffffffe << square); //maybe 0xff...fe ???
		squaresAhead[1][square] = files[square & 7] & (0xfffffffffffffffe >> square);
	}
}

#define COL(x) x%8
#define ROW(x) x>>3
void initTropism() {

	int diag_nw[64] = {
	   0, 1, 2, 3, 4, 5, 6, 7,
	   1, 2, 3, 4, 5, 6, 7, 8,
	   2, 3, 4, 5, 6, 7, 8, 9,
	   3, 4, 5, 6, 7, 8, 9,10,
	   4, 5, 6, 7, 8, 9,10,11,
	   5, 6, 7, 8, 9,10,11,12,
	   6, 7, 8, 9,10,11,12,13,
	   7, 8, 9,10,11,12,13,14
	};

	int diag_ne[64] = {
	   7, 6, 5, 4, 3, 2, 1, 0,
	   8, 7, 6, 5, 4, 3, 2, 1,
	   9, 8, 7, 6, 5, 4, 3, 2,
	  10, 9, 8, 7, 6, 5, 4, 3,
	  11,10, 9, 8, 7, 6, 5, 4,
	  12,11,10, 9, 8, 7, 6, 5,
	  13,12,11,10, 9, 8, 7, 6,
	  14,13,12,11,10, 9, 8, 7
	};


	int bonusDiaDistance[14] = { 5, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int i, j;

	for (i = 0; i < 64; ++i) {
		for (j = 0; j < 64; ++j) {
			distBonus[i][j] = 14 - (abs((COL(i)) - (COL(j))) + abs((ROW(i)) - (ROW(j))));

			qkdist[i][j] = (distBonus[i][j] * 5) / 2;
			rkdist[i][j] = distBonus[i][j] / 2;
			nkdist[i][j] = distBonus[i][j];
			/* bk_dist[i][j] takes into account the numbers of the diagonals */
			bkdist[i][j] = distBonus[i][j] / 2;
			kbdist[i][j] += bonusDiaDistance[abs(diag_ne[i] - diag_ne[j])];
			kbdist[i][j] += bonusDiaDistance[abs(diag_nw[i] - diag_nw[j])];
		}
	}

}

U64 setOccupancy(int index, __int64 bitsInMask, U64 attackMask) {
	U64 occupancy = 0ULL;
	//loop over range of bits within attack mask
	for (int i = 0; i < bitsInMask; i++) {
		// get LS1b index of attack mask
		unsigned long square;
		bitScanForward(&square, attackMask);
		//clear it
		clearBit(attackMask,square);
		//check wheter occupancy is on board, if true populate occupancy
		if (index & squareBB(i)) occupancy |= squareBB(square);
	}
	return occupancy;
};

U64 maskPawnAttacks(int square, int sideToMove) {
	//piece bitboard
	U64 bitboard = squareBB(square);
	//res bitboard
	U64 attacks = 0ULL;
	//printBitBoard(bitboard);
	//white
	if (!sideToMove) {
		if (bitboard & NOTFILE_H) attacks |= (bitboard >> 7);
		if (bitboard & NOTFILE_A) attacks |= (bitboard >> 9);
	}
	//black
	else {
		if (bitboard & NOTFILE_A) attacks |= (bitboard << 7);
		if (bitboard & NOTFILE_H) attacks |= (bitboard << 9);
	}

	//return attacks
	return attacks;
};
U64 maskBishopAttacks(int square) {
	//results
	U64 attacks = 0ULL;
	
	//init ranks & files
	int r, f;

	//initialize target ranks & files
	int tr = square / 8;
	int tf = square & 0x7;

	//mask relevante bishop occupancy bits

	for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= squareBB(r * 8 + f);
	for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= squareBB(r * 8 + f);
	for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= squareBB(r * 8 + f);
	for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= squareBB(r * 8 + f);

	//return attack map
	return attacks;
};
U64 maskRookAttacks(int square) {
	//results
	U64 attacks = 0ULL;

	//init ranks & files
	int r, f;

	//initialize target ranks & files
	int tr = square / 8;
	int tf = square & 0x7;

	//mask relevante bishop occupancy bits

	for (r = tr + 1; r <= 6 ; r++) attacks |= squareBB(r * 8 + tf);
	for (r = tr - 1; r >= 1; r--) attacks |= squareBB(r * 8 + tf);
	for (f = tf + 1; f <= 6 ; f++) attacks |= squareBB(tr * 8 + f);
	for (f = tf - 1; f >= 1 ; f--) attacks |= squareBB(tr * 8 + f);

	//return attack map
	return attacks;
};
U64 bishopAttacksOnTheFly(int square, U64 block) {
	//results
	U64 attacks = 0ULL;

	//init ranks & files
	int r, f;

	//initialize target ranks & files
	int tr = square / 8;
	int tf = square & 0x7;

	//generate bishop attacks

	for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
		U64 possB = squareBB(r * 8 + f);
		attacks |= possB;
		if (block & possB) break;
	}
	for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
		U64 possB = squareBB(r * 8 + f);
		attacks |= possB;
		if (block & possB) break;
	}
	for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
		U64 possB = squareBB(r * 8 + f);
		attacks |= possB;
		if (block & possB) break;
	}
	for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
		U64 possB = squareBB(r * 8 + f);
		attacks |= possB;
		if (block & possB) break;
	}

	//return attack map
	return attacks;
};
U64 rookAttacksOnTheFly(int square, U64 block) {
	//results
	U64 attacks = 0ULL;

	//init ranks & files
	int r, f;

	//initialize target ranks & files
	int tr = square / 8;
	int tf = square & 0x7;

	//mask relevante bishop occupancy bits

	for (r = tr + 1; r <= 7; r++) {
		U64 possB = squareBB(r * 8 + tf);
		attacks |= possB;
		if (block & possB) break;
	}
	for (r = tr - 1; r >= 0; r--) {
		U64 possB = squareBB(r * 8 + tf);
		attacks |= possB;
		if (block & possB) break;
	}
	for (f = tf + 1; f <= 7; f++) {
		U64 possB = squareBB(tr * 8 + f);
		attacks |= possB;
		if (block & possB) break;
	}
	for (f = tf - 1; f >= 0; f--) {
		U64 possB = squareBB(tr * 8 + f);
		attacks |= possB;
		if (block & possB) break;
	}

	//return attack map
	return attacks;
};

void initSlidersAttacks(int bishop) {
	//init bishop & rook masks
	for (int square = 0; square < 64; square++) { //loop over squares
		//init bishop & rook masks
		bishopMasks[square] = maskBishopAttacks(square);
		rookMasks[square] = maskRookAttacks(square);

		//init current mask
		U64 attackMask = bishop ? bishopMasks[square] : rookMasks[square];

		//init relevant occupancy bit cnt
		int relevantBitsCount = popcount(attackMask);

		//init occupancy indexs
		int occupancyIndexs = squareBB(relevantBitsCount);

		//loop over occupancyIndexs
		for(int index = 0; index < occupancyIndexs; index++) {
			if (bishop){ //first case
				//init current occupancy variation
				U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask); 

				//init magic index
				int magicIndex = (occupancy * bishopMagicNumbers[square]) >> (64 - bishopRelevantBits[square]);

				//init bishopAttacks (finally)
				bishopAttacks[square][magicIndex] = bishopAttacksOnTheFly(square, occupancy);
			} //second case
			else {
				//init current occupancy variation
				U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

				//init magic index
				int magicIndex = (occupancy * rookMagicNumbers[square]) >> (64 - rookRelevantBits[square]);

				//init bishopAttacks (finally)
				rookAttacks[square][magicIndex] = rookAttacksOnTheFly(square, occupancy);
			}
		}
	}
}

inline U64 getBishopAttacks(int square, U64 occupancy) {
	occupancy &= bishopMasks[square];
	occupancy *= bishopMagicNumbers[square];
	occupancy >>= 64ULL - bishopRelevantBits[square];
	return bishopAttacks[square][occupancy];
}

inline U64 getRookAttacks(int square, U64 occupancy) {
	occupancy &= rookMasks[square];
	occupancy *= rookMagicNumbers[square];
	occupancy >>= 64ULL - rookRelevantBits[square];
	return rookAttacks[square][occupancy];
}

inline U64 getQueenAttacks(int square, U64 occupancy) {

	/*U64 bishopOccupancies = occupancy;
	U64 rookOccupancies = occupancy;

	bishopOccupancies &= bishopMasks[square];
	bishopOccupancies *= bishopMagicNumbers[square];
	bishopOccupancies >>= 64 - bishopRelevantBits[square];

	rookOccupancies &= rookMasks[square];
	rookOccupancies *= rookMagicNumbers[square];
	rookOccupancies >>= 64 - rookRelevantBits[square];

	return rookAttacks[square][rookOccupancies] | bishopAttacks[square][bishopOccupancies] ;*/
	return getRookAttacks(square, occupancy) | getBishopAttacks(square, occupancy);
}

// random piece keys
U64 pieceKeys[12][64];
//random enPassant keys
U64 enPassantKeys[65];
//random castling keys
U64 castleKeys[16];
//random side key
U64 sideKeys;
//hashTable
extern tt* hashTable;


void initHashKeys() {
	//seed
	state = 1804289383;
	//piece keys
	for (int i = 0; i < 12; i++) {
		for (int k = 0; k < 64; k++) {
			//init the key
			pieceKeys[i][k] = getRandomNumber64();
			//printf("0x%llx\n", pieceKeys[i][k]);
		}
	}
	//en passant
	for (int i = 0; i < 64; i++) {
		enPassantKeys[i] = getRandomNumber64();
	}
	enPassantKeys[64] = 0;
	//castle
	for (int i = 0; i < 16; i++) {
		castleKeys[i] = getRandomNumber64();
	}
	//side
	sideKeys = getRandomNumber64();
}

inline void wipeTT() {
	for (int i = 0; i < hashSize; i++) {
		hashTable[i].wipe();
	}
}

inline void tt::wipe() {
	key = 0;
	depth = 0;
	flags = 0;
	score = 0;
	move = 0;
}

#define NOENTRY 100000
inline int readHashEntry(int key, int alpha, int beta, int depth) {
	//create a TT instance to point to hash entry
	
	unsigned int target = key & (hashSize -1);

	//std::cout << "trying to access hashTable at " << std::hex <<key<<" % "<<hashSize<<" = "<< target << std::endl;

	tt *hashEntry = &hashTable[target];

	// make sure we're dealing with the exact position we need
	if (hashEntry->key == key)
	{
		// make sure that we match the exact depth our search is now at
		if (hashEntry->depth >= depth)
		{

			int score = hashEntry->score;
			if (score < -mateScore)score += ply;
			if (score >  mateScore)score -= ply;


			// match the exact (PV node) score 
			if (hashEntry->flags == hashEXACT)
				// return exact (PV node) score
				return score;

			// match alpha (fail-low node) score
			if ((hashEntry->flags == hashALPHA) &&
				(score <= alpha))
				// return alpha (fail-low node) score
				return alpha;

			// match beta (fail-high node) score
			if ((hashEntry->flags == hashBETA) &&
				(score >= beta))
				// return beta (fail-high node) score
				return beta;
		}
	}
	// if hash entry doesn't exist
	return NOENTRY;
}

inline void writeHashEntry(int key, int score, int depth, moveInt move, int hashFlag) {
	// create a TT instance pointer to particular hash entry storing
	// the scoring data for the current board position if available
	unsigned int target = key & (hashSize - 1);
	tt* hash_entry = &hashTable[target];

#if true
	if (depth < hash_entry->depth || (depth == hash_entry->depth && hash_entry->flags != hashALPHA && hashFlag == hashALPHA)) return;
	//if (depth < hash_entry->depth) return;
	//if (depth == hash_entry->depth && hash_entry->flags == hashEXACT)return;
#else
	if (depth < hash_entry->depth || (depth < hash_entry->depth + 1 && hash_entry->flags != hashALPHA && hashFlag == hashALPHA)) return;
#endif
	score -= ply * (score < -mateScore);
	score += ply * (score > mateScore);

	

	// write hash entry data 
	hash_entry->key = key;
	hash_entry->score = score;
	hash_entry->flags = hashFlag;
	hash_entry->depth = depth;
	hash_entry->move = move;
}

inline tt* getEntry(int key) {
	unsigned int target = key & (hashSize - 1);
	return &hashTable[target];
}

inline void ageTT() {
	for (int i = 0; i < hashSize; i++) {
		if (hashTable[i].depth == 1) {
			hashTable[i].wipe();
		}
		hashTable[i].depth--;
	}
}