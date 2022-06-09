#pragma once

class Tester {
private:
	int firstDepth = 0;
	int from = 0, to = 0;
	char extra = ' ';
public:
	int perft(int depth);
};


int Tester::perft(int depth)
{
	if (firstDepth < depth) firstDepth = depth;
	if (depth == 0) return 1;

	bitset<64> bitboard = turn == white ? Wpieces : Bpieces;

	int numMoves = 0;

	for (int origin = 0; origin < 64; origin++) {
		if (!bitboard.test(origin)) continue;

		Colour c = getPieceColour(origin);
		Type t = getPieceType(origin);

		bitset<64> attacks = getLegalMoves(static_cast<enumSquare>(origin), c, t)
			             | getLegalCaptures(static_cast<enumSquare>(origin), c, t);

		if (attacks.none()) continue;
		for (int target = 0; target < 64; target++) {
			if (!attacks.test(target)) continue;

			int options = 1;
			if (((turn == black && target < 8) || (turn == white && target >= 56)) && t == p)
				options = 4;

			for (int promotion = 0; promotion < options; promotion++)
			{
				bitset<64> pEnPassentTargets = EPTargets;
				bitset<4> pCastleRights = castlingRights;
				
				Promotion promo = options == 1 ? toNone : static_cast<Promotion>(promotion);

				makeMove(static_cast<enumSquare>(origin), static_cast<enumSquare>(target), c, t, promo);

				if (depth == firstDepth) {
					from = origin; to = target;
					if (options == 4)
						extra = promotion == 0 ? 'n' : promotion == 1 ? 'b' : promotion == 2 ? 'r' : 'q';
					else
						extra = ' ';
				}
				numMoves += perft(depth - 1);

				unmakeMove();
				castlingRights = pCastleRights;
				EPTargets = pEnPassentTargets;
			}
		}
	}
	//prints out the number of moves after each first move
	if (depth == firstDepth - 1) 
		cout << char(from % 8 + 97) << from / 8 + 1 << char(to % 8 + 97) << to / 8 + 1 << extra << ": " << numMoves << endl;
	
	return numMoves;
}