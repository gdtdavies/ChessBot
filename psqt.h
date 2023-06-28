#pragma once

class psqt {

private:
	const int pawnScoreBoard[64] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		 50,  50,  50,  50,  50,  50,  50,  50,
		 10,  10,  20,  30,  30,  20,  10,  10,
		  5,   5,  10,  25,  25,  10,   5,   5,
		  0,   0,   0,  20,  20,   0,   0,   0,
		  5,  -5, -10,   0,   0, -10,  -5,   0,
		  5,  10,  10, -20, -20,  10,  10,   5,
	};
	const int knightScoreBoard[64] = {
			-50, -40, -30, -30, -30, -30, -40, -50,
			-40, -20,   0,   0,   0,   0, -20, -40,
			-30,   0,  10,  15,  15,  20,  10,  10,
			-30,   5,  15,  20,  20,  15,   5, -30,
			-30,   0,  15,  20,  20,  15,   0, -30,
			-30,   5,  10,  15,  15,  10,   5, -30,
			-40, -20,   0,   5,   5,   0, -20, -40,
			-50, -40, -30, -30, -30, -30, -40, -50
	};
	const int bishopScoreBoard[64] = {
			-20, -10, -10, -10, -10, -10, -10, -10,
			-10,   0,   0,   0,   0,   0,   0, -10,
			-10,   0,   5,  10,  10,   5,   0, -10,
			-10,   5,   5,  10,  10,   5,   5, -10,
			-10,   0,  10,  10,  10,  10,   0, -10,
			-10,  10,  10,  10,  10,  10,  10, -10,
			-10,   5,   0,   0,   0,   0,   5, -10,
			-20, -10, -10, -10, -10, -10, -10, -20
	};
	const int rookScoreBoard[64] = {
			  0,   0,   0,   0,   0,   0,   0,   0,
			  5,   0,   0,   0,   0,   0,   0,   5,
			 -5,   0,   0,   0,   0,   0,   0,  -5,
			 -5,   0,   0,   0,   0,   0,   0,  -5,
			 -5,   0,   0,   0,   0,   0,   0,  -5,
			 -5,   0,   0,   0,   0,   0,   0,  -5,
			 -5,   0,   0,   0,   0,   0,   0,  -5,
			  0,   0,   0,   5,   5,   0,   0,   0
	};
	const int queenScoreBoard[64] = {
			-20, -10, -10,  -5,  -5, -10, -10, -20,
			-10,   0,   0,   0,   0,   0,   0, -10,
			-10,   0,   5,   5,   5,   5,   0, -10,
			 -5,   0,   5,   5,   5,   5,   0,  -5,
			  0,   0,   5,   5,   5,   5,   0,   0,
			-10,   5,   5,   5,   5,   5,   5, -10,
			-10,   0,   5,   0,   0,   5,   0, -10,
			-20, -10, -10,  -5,  -5, -10, -10, -20
	};
	const int kingScoreBoard[64] = {
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-20, -30, -30, -40, -40, -30, -30, -20,
			-10, -20, -20, -20, -20, -20, -20, -10,
			 20,  20,   0,   0,   0,   0,  20,  20,
			 20,  30,  10,   0,   0,  10,  30,  20
	};

public:
	int getPieceScore(int square, Type t);
};

int psqt::getPieceScore(int square, Type t) {
	switch (t) {
	case p:
		return pawnScoreBoard[square];
	case n:
		return knightScoreBoard[square];
	case b:
		return bishopScoreBoard[square];
	case r:
		return rookScoreBoard[square];
	case q:
		return queenScoreBoard[square];
	case k:
		return kingScoreBoard[square];
	}
}