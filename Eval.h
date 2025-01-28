#pragma once
#include "psqt.h"
#include <math.h>

class Evaluator {
private:
	int rootDepth = 0;
	Move bestMove;
	psqt squareTable;
	vector<float> rootScores;
	vector<Move> rootMoves;

	float getPieceSquareTable();
	float getPieceMobility(bitset<64> WpawnAttacks, bitset<64> BpawnAttacks);
	float evalPawnStructure(int WpPerFile[8], int BpPerFile[8]);
	float evalRookStructure(int WpPerFile[8], int BpPerFile[8]);
	float evalKnightStructure(bitset<64> WpawnAttacks, bitset<64> BpawnAttacks);
	float quiesce(float alpha, float beta);
	Move findBestMove();
public:
	double evaluate(int depth);
	float minimax(int depth, float alpha, float beta);
	void reset();

	Move getBestMove();
};

float Evaluator::getPieceSquareTable() {
	float score = 0;
	for (int sq = 0; sq < 64; sq++) {
		if (!Occupied.test(sq)) continue;
		Type t = getPieceType(sq);
		Colour c = getPieceColour(sq);
		if (c == white)
			score += squareTable.getPieceScore(63 - sq, t);
		else
			score -= squareTable.getPieceScore(sq, t);
	}
	return score;
}

float Evaluator::getPieceMobility(bitset<64> WpawnAttacks, bitset<64> BpawnAttacks) {
	int Wmobility = 0, Bmobility = 0;

	for (int sq = 0; sq < 64; sq++) {
		Type t = getPieceType(sq);
		if (Wpieces.test(sq)) {
			bitset<64> attacks = mg.getLegalMoves(static_cast<enumSquare>(sq), white, t)
				             | mg.getLegalCaptures(static_cast<enumSquare>(sq), white, t);
			//When calculating knight mobility, it is advisable to omit squares 
			//controlled by enemy pawns
			if (Wn.test(sq))
				attacks = attacks & ~BpawnAttacks;
			Wmobility += attacks.count();
		}
		else if (Bpieces.test(sq)) {
			bitset<64> attacks = mg.getLegalMoves(static_cast<enumSquare>(sq), black, t)
				             | mg.getLegalCaptures(static_cast<enumSquare>(sq), black, t);
			if (Bn.test(sq))
				attacks =  attacks & ~WpawnAttacks;
			Bmobility += attacks.count();
		}
	}

	return Wmobility - Bmobility;
}

float Evaluator::evalPawnStructure(int WpPerFile[8], int BpPerFile[8]) {
	//=doubled pawns===========================================================|
	//a pawn is doubled when there is another pawn of the same colour on the
	//same file.
	int Wdoubled = 0, Bdoubled = 0;

	for (int i = 0; i < 8; i++) {
		if (WpPerFile[i] > 1)
			Wdoubled += WpPerFile[i] - 1;
		if (BpPerFile[i] > 1)
			Bdoubled += BpPerFile[i] - 1;
	}
	int doubled = -(Wdoubled - Bdoubled);

	//=isolated pawns==========================================================|
	//a pawn is isolated when there are no pawns of the same colour in adjacent
	//files
	int Wisolated = 0, Bisolated = 0;

	for (int i = 0; i < 8; i++) {
		if (WpPerFile[i] != 0)
		{
			if (i == 0 && WpPerFile[1] == 0) //left most file
				Wisolated += WpPerFile[i];
			else if (i == 7 && WpPerFile[6] == 0) //right most file
				Wisolated += WpPerFile[i];
			else if (WpPerFile[i - 1] == 0 && WpPerFile[i + 1] == 0) //all other files
				Wisolated += WpPerFile[i];
		}
		if (BpPerFile[i] != 0)
		{
			if (i == 0 && BpPerFile[1] == 0) //left most file
				Bisolated += BpPerFile[i];
			else if (i == 7 && BpPerFile[6] == 0) //right most file
				Bisolated += BpPerFile[i];
			else if (BpPerFile[i - 1] == 0 && BpPerFile[i + 1] == 0) //all other files
				Bisolated += BpPerFile[i];
		}
	}
	int isolated = -(Wisolated - Bisolated);

	//=blocked pawns===========================================================|
	//a pawn is blocked when it has no available moves or captures
	int Wblocked = 0, Bblocked = 0;

	for (int sq = 8; sq < 56; sq++) {
		if (Wp.test(sq) && Occupied.test(sq + 8))
		{
			if (sq % 8 == 0 && !Bpieces.test(sq + 9)) Wblocked++;
			if (sq % 8 == 7 && !Bpieces.test(sq + 7)) Wblocked++;

			if (sq % 8 > 0 && sq % 8 < 7)
				if (!Bpieces.test(sq + 7) && !Bpieces.test(sq + 9)) Wblocked++;
			//extra penalty if the center pawns are blocked on their home squares
			if ((sq == 11 || sq == 12)) {
				if (!Bpieces.test(sq + 7) && !Bpieces.test(sq + 9))
					Wblocked++;
			}
		}
		if (Bp.test(sq) && Occupied.test(sq - 8))
		{
			if (sq % 8 == 0 && !Bpieces.test(sq - 7)) Bblocked++;
			if (sq % 8 == 7 && !Bpieces.test(sq - 9)) Bblocked++;

			if (sq % 8 > 0 && sq % 8 < 7)
				if (!Wpieces.test(sq - 9) && !Wpieces.test(sq - 7)) Bblocked++;
			//extra penalty if the center pawns are blocked on their home squares
			if ((sq == 51 || sq == 52)) {
				if (!Wpieces.test(sq - 7) && !Wpieces.test(sq - 9))
					Bblocked++;
			}
		}
	}
	int blocked = -(Wblocked - Bblocked);

	//=passed pawns============================================================|
	//a pawn is passed if it has no opponant pawns in adjacent files

	int Wpassed = 0, Bpassed = 0;

	for (int file = 0; file < 8; file++) {
		if (file == 0) {
			if (WpPerFile[file] != 0) 
				if (WpPerFile[0] != 0 && BpPerFile[1] == 0)
					Wpassed++;
			if (BpPerFile[file] != 0)
				if (BpPerFile[0] != 0 && WpPerFile[1] == 0)
					Bpassed++;
		}
		else if (file == 7) {
			if (WpPerFile[file] != 0)
				if (WpPerFile[7] != 0 && BpPerFile[6] == 0)
					Wpassed++;
			if (BpPerFile[file] != 0)
				if (BpPerFile[7] != 0 && WpPerFile[6] == 0)
					Bpassed++;
		}
		else {
			if (WpPerFile[file] != 0)
				if (BpPerFile[file - 1] == 0 && BpPerFile[file + 1] == 0)
					Wpassed++;
			if (BpPerFile[file] != 0)
				if (WpPerFile[file - 1] == 0 && WpPerFile[file + 1] == 0)
					Bpassed++;
		}
	}

	int passed = Wpassed - Bpassed;

	return (doubled + isolated + blocked + passed);
}

float Evaluator::evalRookStructure(int WpPerFile[8], int BpPerFile[8]) {
	float wScore = 0,
		bScore = 0;
	int WrPerFile[8] = { 0, 0, 0, 0, 0, 0, 0, 0 },
		BrPerFile[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	//bonus for rook on 7th and to a lesser extent 8th
	wScore += (Wr & Rank7).count();
	wScore += (Wr & Rank8).count() * 0.5;

	bScore += (Br & Rank2).count();
	bScore += (Br & Rank1).count() * 0.5;

	for (int sq = 0; sq < 64; sq++) {
		if (Wr.test(sq)) {
			WrPerFile[sq % 8]++;
			if (WpPerFile[sq % 8] == 0) {
				//fully open file
				if (BpPerFile[sq % 8] == 0) wScore++;
				//semi open file
				else if (BpPerFile[sq % 8] == 1) wScore += 0.5;
			}
			//Small bonus for a rook with enemy queen on the same file
			for (int i = sq % 8; i < 64; i += 8) {
				if (i == sq) continue;
				if (!Bq.test(i)) continue;
				wScore += 0.1;
				break;
			}
		}
		else if (Br.test(sq)) {
			BrPerFile[sq % 8]++;
			if (BpPerFile[sq % 8] == 0) {
				//fully open file
				if (WpPerFile[sq % 8] == 0) bScore++;
				//semi open file
				else if (WpPerFile[sq % 8] == 1) bScore += 0.5;
			}
			//Small bonus for a rook with enemy queen on the same file
			for (int i = sq % 8; i < 64; i += 8) {
				if (i == sq) continue;
				if (!Wq.test(i)) continue;
				bScore += 0.1;
				break;
			}
		}
	}
	//find out if the rooks are doubled, give a bonus if they are
	for (int file = 0; file < 8; file++) {
		if (WrPerFile[file] == 2) wScore *= 2;
		if (BrPerFile[file] == 2) bScore *= 2;
	}


	return wScore - bScore;
}

float Evaluator::evalKnightStructure(bitset<64> WpAttacks, bitset<64> BpAttacks) {
	float wScore = 0,
		bScore = 0;
	//=outposts================================================================|
	for (int sq = 33; sq < 47; sq++) {
		if (sq % 8 == 0 || sq % 8 == 7) continue;
		//if a knight is in the opponants half and protected by a pawn and not
		//attacked by a pawn, it is an outpost
		if (!Wn.test(sq)) continue; //continue if not a knight
		//continue if not protected
		if (!(Wp.test(sq - 9) || Wp.test(sq - 7))) continue;
		//continue if attacked by pawn
		if (Bp.test(sq + 7) || Bp.test(sq + 9)) continue;
		wScore++;
	}
	for (int sq = 17; sq < 31; sq++) {
		if (sq % 8 == 0 || sq % 8 == 7) continue;
		if (!Bn.test(sq)) continue;
		if (!(Bp.test(sq + 9) || Bp.test(sq + 7))) continue;
		if (Wp.test(sq - 7) || Wp.test(sq - 9)) continue;
		bScore++;
	}

	//=protected by pawn=======================================================|
	// Marginal bonus for a knight defended by a pawn
	wScore += (Wn & WpAttacks).count() * 0.25;
	bScore += (Bn & BpAttacks).count() * 0.25;

	return wScore - bScore;
}

double Evaluator::evaluate(int depth) {
	if (isStaleMate(turn) || isDraw()) return 0;
	if (isCheckMate(turn)) return -99999 - depth; // so that it proritises quicker mates

	int WpPerFile[8] = { 0, 0, 0, 0, 0, 0, 0, 0 },
		BpPerFile[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int file = 0; file < 8; file++) {
		WpPerFile[file] = (Wp & Files[file]).count();
		BpPerFile[file] = (Bp & Files[file]).count();
	}

	bitset<64> WpawnAttacks(0), BpawnAttacks(0);
	for (int sq = 8; sq < 56; sq++) {
		if (Wp.test(sq)) {
			if (sq % 8 > 0) WpawnAttacks.set(sq + 7, 1);
			if (sq % 8 < 7) WpawnAttacks.set(sq + 9, 1);
		}
		else if (Bp.test(sq)) {
			if (sq % 8 > 0) BpawnAttacks.set(sq - 9, 1);
			if (sq % 8 < 7) BpawnAttacks.set(sq - 7, 1);
		}
	}

	int multiplier = turn == white ? 1 : -1;
	float material
		= 9.00f * (int)(Wq.count() - Bq.count())
		+ 5.00f * (int)(Wr.count() - Br.count())
		+ 3.00f * (int)(Wb.count() - Bb.count())
		+ 3.00f * (int)(Wn.count() - Bn.count())
		+ 1.00f * (int)(Wp.count() - Bp.count());
	float pieceSquareTable = getPieceSquareTable();
	float mobility = getPieceMobility(WpawnAttacks, BpawnAttacks);
	float pawnStructure = evalPawnStructure(WpPerFile, BpPerFile);
	float rookStructure = evalRookStructure(WpPerFile, BpPerFile);
	float knightSturcture = evalKnightStructure(WpawnAttacks, BpawnAttacks);

	float score = material
		+ 0.005f * pieceSquareTable
		+ 0.010f * mobility
		+ 0.150f * pawnStructure
		+ 0.100f * rookStructure
		+ 0.050f * knightSturcture;

	return score * multiplier;
}

float Evaluator::minimax(int depth, float alpha, float beta) {
	
	if (depth <= 0) return quiesce(alpha, beta); //return evaluate(depth);

	vector<Move> attacks = mg.getAllMoves(false);
	if (attacks.empty()) return evaluate(depth);

	if (depth > rootDepth) 
		rootDepth = depth;
	
	for (Move attack : attacks) {

		//there is no situation where it is better to promote to a bishop or a rook rather than a queen (bar the rare case of stalemate)
		if (attack.getPromotion() == toB || attack.getPromotion() == toR) continue;

		bitset<64> pEnPassentTargets = EPTargets;
		bitset<4> pCastleRights = castlingRights;

		bitset<64> rooks = Wr;

		makeMove(attack.getOrigin(), attack.getDestination(), attack.getColour(), attack.getType(), attack.getPromotion());

		/*if (attack.getOrigin() == 63)
			cout << "now" << endl;*/
			
		float score = -minimax(depth - 1, -beta, -alpha);


		unmakeMove();
		/*if (attack.getOrigin() == 63)
			cout << "now" << endl;*/

		/*if (Wr.test(63))
			cout << "now" << endl;*/


		castlingRights = pCastleRights;
		EPTargets = pEnPassentTargets;
		
		if (depth == rootDepth) {
			rootMoves.push_back(attack);
			rootScores.push_back(score);
			cout << attack.getMoveCode() << " " << score << endl;
		}

		if (score >= beta) return beta;
		if (score > alpha) alpha = score;

	}	
	if (depth == rootDepth) 
		bestMove = findBestMove();

	return alpha;
}

float Evaluator::quiesce(float alpha, float beta) {
	float stand_pat = evaluate(0);
	if (stand_pat >= beta) return beta;
	if (stand_pat > alpha) alpha = stand_pat;

	vector<Move> captures = mg.getAllMoves(true);

	if (captures.empty()) return alpha;

	for (Move capture : captures) {
		bitset<64> pEnPassentTargets = EPTargets;
		bitset<4> pCastleRights = castlingRights;

		makeMove(capture.getOrigin(), capture.getDestination(), capture.getColour(), capture.getType(), capture.getPromotion());
		float score = -quiesce(-beta, -alpha);
		unmakeMove();

		castlingRights = pCastleRights;
		EPTargets = pEnPassentTargets;

		if (score >= beta) 
			return beta;
		if (score > alpha) 
			alpha = score;

	}
	return alpha;
}

Move Evaluator::findBestMove() {
	int bestMove = 0;
	for (int i = 1; i < rootScores.size(); i++) {
		if (rootScores.at(bestMove) < rootScores.at(i))
			bestMove = i;
	}
	return rootMoves.at(bestMove);
}

void Evaluator::reset() {
	rootDepth = 0;
	rootScores.clear();
	rootMoves.clear();
}

Move Evaluator::getBestMove() {
	return bestMove;
}
