#pragma once

class Evaluator {
private:
	Move bestMove;
public:
	float evaluate();
	float minimax(int depth, float alpha, float beta);

	Move getBestMove();
};

float Evaluator::evaluate() {
	if (isStaleMate(turn)) return 0;
	if (isCheckMate(turn)) return -INFINITY;

	float material
		= 9.00 * (int)(Wq.count() - Bq.count())
		+ 5.00 * (int)(Wr.count() - Br.count())
		+ 3.00 * (int)(Wb.count() - Bb.count())
		+ 3.00 * (int)(Wn.count() - Bn.count())
		+ 1.00 * (int)(Wp.count() - Bp.count());
	int multiplier = turn == white ? 1 : -1;
	return material * multiplier;
}

float Evaluator::minimax(int depth, float alpha, float beta) {
	if (depth == 0) return evaluate();
	bitset<64> bitboard = turn == white ? Wpieces : Bpieces;

	vector<Move> attacks = getAllMoves(false);
	
	for (Move attack : attacks) {

		bitset<64> pEnPassentTargets = EPTargets;
		bitset<4> pCastleRights = castlingRights;

		makeMove(attack.getOrigin(), attack.getDestination(), attack.getColour(), attack.getType(), attack.getPromotion());
		Move move = movesMade.at(moveCount);
		float score = -minimax(depth - 1, -beta, -alpha);
		
		unmakeMove();
		castlingRights = pCastleRights;
		EPTargets = pEnPassentTargets;

		if (score >= beta)
			return beta;
		if (score > alpha) {
			alpha = score;
			bestMove = move;
		}

	}
	return alpha;
}

Move Evaluator::getBestMove() {
	return bestMove;
}
