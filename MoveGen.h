class moveGenerator {
private:
	vector<Move> orderMoves(vector<Move> moves);
	bitset<64> getSquaresAttackedByPawn(Colour c);
public:
	bitset<64> getLegalMoves(enumSquare sq, Colour c, Type t);
	bitset<64> getLegalCaptures(enumSquare sq, Colour c, Type t);
	vector<Move> getAllMoves(bool capturesOnly);
};

bitset<64> moveGenerator::getLegalMoves(enumSquare sq, Colour c, Type t) {
	bitset<64> moves(0), mask(0);
	switch (t) {
	case p:
		if (c == white) {
			if (Occupied.test(sq + 8)) break;
			makeMove(sq, static_cast<enumSquare>(sq + 8), c, t);
			if (!isCheck(c))
				moves.set(sq + 8, 1);
			unmakeMove();
			if (sq < 16 && !Occupied.test(sq + 16)) {
				makeMove(sq, static_cast<enumSquare>(sq + 16), c, t);
				if (!isCheck(c)) moves.set(sq + 16, 1);
				unmakeMove();
			}
		}
		else {
			if (Occupied.test(sq - 8)) break;
			makeMove(sq, static_cast<enumSquare>(sq - 8), c, t);
			if (!isCheck(c))
				moves.set(sq - 8, 1);
			unmakeMove();
			if (sq >= 48 && !Occupied.test(sq - 8) && !Occupied.test(sq - 16)) {
				makeMove(sq, static_cast<enumSquare>(sq - 16), c, t);
				if (!isCheck(c))
					moves.set(sq - 16, 1);
				unmakeMove();
			}
		}
		return moves;
	case n:
		mask = nA.getKnightAttacks(sq) & ~Occupied;
		break;
	case b:
		mask = sA.getBishopAttacks(Occupied, sq) & ~Occupied;
		break;
	case r:
		mask = sA.getRookAttacks(Occupied, sq) & ~Occupied;
		break;
	case q:
		mask = sA.getQueenAttacks(Occupied, sq) & ~Occupied;
		break;
	case k:
		mask = kA.getKingAttacks(sq) & ~Occupied;
		if (sq == 4) {
			for (int i = 0; i < 2; i++) {
				int negative = i % 2 == 0 ? -1 : 1;
				if (castlingRights.test(i)) {
					if (!Occupied.test(sq + negative) && !Occupied.test(sq + negative * 2)) {
						if (!isCheck(c)) {
							makeMove(sq, static_cast<enumSquare>(sq + negative), c, t);
							if (!isCheck(c)) {
								unmakeMove();
								makeMove(sq, static_cast<enumSquare>(sq + negative * 2), c, t);
								if (!isCheck(c)) {
									if (negative == 1 || !Occupied.test(sq - 3))
										moves.set(sq + negative * 2);
								}
							}
							unmakeMove();
						}
					}
				}
			}
		}
		else if (sq == 60) {
			for (int i = 2; i < 4; i++) {
				if (castlingRights.test(i)) {
					int negative = i % 2 == 0 ? -1 : 1;
					if (!Occupied.test(sq + negative) && !Occupied.test(sq + negative * 2)) {
						if (!isCheck(c)) {
							makeMove(sq, static_cast<enumSquare>(sq + negative), c, t);
							if (!isCheck(c)) {
								unmakeMove();
								makeMove(sq, static_cast<enumSquare>(sq + negative * 2), c, t);
								if (!isCheck(c)) {
									if (negative == 1 || !Occupied.test(sq - 3))
										moves.set(sq + negative * 2);
								}
							}
							unmakeMove();
						}
					}
				}
			}
		}
		break;
	}

	for (int sq2 = 0; sq2 < 64; sq2++) {
		if (!mask.test(sq2)) continue;

		makeMove(sq, static_cast<enumSquare>(sq2), c, t);
		if (!isCheck(c))
			moves.set(sq2, 1);
		unmakeMove();
	}

	return moves;
}

bitset<64> moveGenerator::getLegalCaptures(enumSquare sq, Colour c, Type t) {
	bitset<64> captures(0), mask(0);
	bitset<64> opponentBB = c == white ? Bpieces : Wpieces;
	switch (t) {
	case p:
		mask = pA.getPawnAttacks(sq, c) & opponentBB;
		if (sq % 8 > 0) {
			if (c == white) {
				if ((32 <= sq && sq < 40) && EPTargets.test(sq + 7)) {
					makeMove(sq, static_cast<enumSquare>(sq + 7), c, t);
					if (!isCheck(c))
						captures.set(sq + 7, 1);
					unmakeMove();
				}
			}
			else {
				if ((24 <= sq && sq < 32) && EPTargets.test(sq - 9)) {
					makeMove(sq, static_cast<enumSquare>(sq - 9), c, t);
					if (!isCheck(c))
						captures.set(sq - 9, 1);
					unmakeMove();
				}
			}
		}
		if (sq % 8 < 7) {
			if (c == white) {
				if ((32 <= sq && sq < 40) && EPTargets.test(sq + 9)) {
					makeMove(sq, static_cast<enumSquare>(sq + 9), c, t);
					if (!isCheck(c))
						captures.set(sq + 9, 1);
					unmakeMove();
				}
			}
			else {
				if ((24 <= sq && sq < 32) && EPTargets.test(sq - 7)) {
					makeMove(sq, static_cast<enumSquare>(sq - 7), c, t);
					if (!isCheck(c))
						captures.set(sq - 7, 1);
					unmakeMove();
				}
			}
		}
		break;
	case n:
		mask = nA.getKnightAttacks(sq) & opponentBB;
		break;
	case b:
		mask = sA.getBishopAttacks(Occupied, sq) & opponentBB;
		break;
	case r:
		mask = sA.getRookAttacks(Occupied, sq) & opponentBB;
		break;
	case q:
		mask = sA.getQueenAttacks(Occupied, sq) & opponentBB;
		break;
	case k:
		mask = kA.getKingAttacks(sq) & opponentBB;
		break;
	}

	for (int sq2 = 0; sq2 < 64; sq2++) {
		if (!mask.test(sq2)) continue;

		makeMove(sq, static_cast<enumSquare>(sq2), c, t);
		if (!isCheck(c))
			captures.set(sq2, 1);
		unmakeMove();
	}

	return captures;
}

bitset<64> moveGenerator::getSquaresAttackedByPawn(Colour c) {
	bitset<64> pawns = c == white ? Wp : Bp;
	bitset<64> attacked;
	for (int sq = 0; sq < 64; sq++) {
		if (!pawns.test(sq)) continue;
		attacked |= pA.getPawnAttacks(sq, c);
	}
	return attacked;
}

vector<Move> moveGenerator::orderMoves(vector<Move> moves) {
	for (int i = 0; i < moves.size(); i++) {
		//taking a more valuable piece is usually good
		moves.at(i).score += 10 * moves.at(i).getTakeValue();
		//promoting is likely to be good
		moves.at(i).score += moves.at(i).getPromotionValue();
		//moving a piece to a square attacked by an enemy pawn is usually bad
		if (getSquaresAttackedByPawn(turn == white ? black : white).test(moves.at(i).getDestination()))
			moves.at(i).score -= moves.at(i).getOriginValue();
	}
	std::sort(moves.begin(), moves.end());

	return moves;
}

vector<Move> moveGenerator::getAllMoves(bool capturesOnly) {
	vector<Move> moves;

	bitset<64> bitboard = turn == white ? Wpieces : Bpieces;
	for (int sq = 0; sq < 64; sq++) {
		if (!bitboard.test(sq)) continue;

		Type t = getPieceType(sq);
		Colour c = turn == white ? white : black;

		bitset<64> attacks = getLegalCaptures(static_cast<enumSquare>(sq), c, t);

		if (!capturesOnly)
			attacks |= getLegalMoves(static_cast<enumSquare>(sq), c, t);

		for (int target = 0; target < 64; target++) {
			if (!attacks.test(target)) continue;
			if (((turn == black && target < 8) || (turn == white && target >= 56)) && t == p) {
				for (int promotion = 0; promotion < 4; promotion++)
				{
					Move move = Move(static_cast<enumSquare>(sq), static_cast<enumSquare>(target), c, t, static_cast<Promotion>(promotion));
					if ((~bitboard & Occupied).test(target))
						move.setTakenType(getPieceType(target));
					makeMove(move.getOrigin(), move.getDestination(), c,t, move.getPromotion());
					move.isCheck = isCheck(turn);
					if (move.isCheck)
						move.isMate = isCheckMate(turn);
					unmakeMove();

					moves.push_back(move);
				}
			}
			else {
				Move move = Move(static_cast<enumSquare>(sq), static_cast<enumSquare>(target), c, t);
				if ((~bitboard & Occupied).test(target))
					move.setTakenType(getPieceType(target));

				makeMove(move.getOrigin(), move.getDestination(), c, t, toNone);
				move.isCheck = isCheck(turn);
				if (move.isCheck)
					move.isMate = isCheckMate(turn);
				else
					move.isDraw = isStaleMate(turn) || isDraw();
				moves.push_back(move);
				unmakeMove();
			}
		}
	}

	return orderMoves(moves);
}
