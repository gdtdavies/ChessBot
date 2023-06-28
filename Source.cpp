#include<iostream>
#include<string>
#include<bitset>
#include<vector>
#include<chrono>
#include<algorithm>

//--- OpenGL ---
#include "GL\glew.h"
#include "GL\wglew.h"
#pragma comment(lib, "glew32.lib")
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "GL\freeglut.h"
//--------------

using namespace std;

const enum Colour : int { white, black, NA };
const enum Type : int {p, r, n, b, q, k, None};
const enum Promotion : int {toN, toB, toR, toQ, toNone};
const enum enumSquare : int {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8, null
};

bool Checkmate = false, Stalemate = false, Draw = false;
bool Wcheck = false, Bcheck = false;

#include "Bitboards.h"
#include "LookupBitBoards/pawn.h"
#include "LookupBitBoards/knight.h"
#include "LookupBitBoards/king.h"
#include "LookupBitBoards/slidingPieces.h"
#include "Move.h"


int screenWidth = 1000, screenHeight = 1000;
double sqW, sqH;

glm::mat4 ViewMatrix, ProjectionMatrix;

Colour turn;

int HMcounter = 0, FMcounter = 0;

enumSquare originSq = null, targetSq = null;

pawnAttacks pA;
knightAttacks nA;
slidingAttacks sA;
kingAttacks kA;

vector<Move> movesMade;

bitset<64> attackBB(0);

bool isPromoting = false;

Promotion promotionChoice = toNone;

//-------------------------------------------------------------------------------------------------
void mouseCallback(int button, int state, int x, int y);

void makeMove(enumSquare origin, enumSquare target, Colour c, Type t, Promotion promoteTo = toNone);
void unmakeMove();


bool isCheck(Colour colourInCheck);
bool isCheckMate(Colour colourInCheck);
bool isStaleMate(Colour colourToMove);

Colour getPieceColour(int sq);
Type getPieceType(int sq);
int getFirstOfRank(int sq);
int getlastOfRank(int sq);

bool isCheck(Colour colourInCheck);
bool isCheckMate(Colour colourInCheck);
bool isStaleMate(Colour colourToMove);
bool isDraw();

void loadFromFen(string fen);

#include "GUI.h"
GUI gui;
#include "TranspositionTable.h"
TranspositionTable TT;
#include "MoveGen.h"
moveGenerator mg;
#include "Eval.h"
Evaluator evaluator;
#include "Testing.h"

//=================================<<--------------------------->>================================//
//=================================<< START OF OPENGL FUNCTIONS >>================================//
//=================================<<--------------------------->>================================//

void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0); //sets the clear colour to black
	glLineWidth(3.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void display() {
	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT);

	ViewMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));

	glEnable(GL_BLEND);

	gui.drawBoard();

	if (originSq != null) {
		gui.drawSelectedSq(originSq / 8, originSq % 8);
		gui.drawAttacks(attackBB);
	}

	for (int sq = 0; sq < 64; sq++) {
		if (!Occupied.test(sq)) continue;

		int rank = sq / 8;
		int file = sq % 8;

		if      (Wp.test(sq)) gui.drawPiece(rank, file, white, p);
		else if (Bp.test(sq)) gui.drawPiece(rank, file, black, p);
		else if (Wr.test(sq)) gui.drawPiece(rank, file, white, r);
		else if (Br.test(sq)) gui.drawPiece(rank, file, black, r);
		else if (Wn.test(sq)) gui.drawPiece(rank, file, white, n);
		else if (Bn.test(sq)) gui.drawPiece(rank, file, black, n);
		else if (Wb.test(sq)) gui.drawPiece(rank, file, white, b);
		else if (Bb.test(sq)) gui.drawPiece(rank, file, black, b);
		else if (Wq.test(sq)) gui.drawPiece(rank, file, white, q);
		else if (Bq.test(sq)) gui.drawPiece(rank, file, black, q);
		else if (Wk.test(sq)) gui.drawPiece(rank, file, white, k);
		else if (Bk.test(sq)) gui.drawPiece(rank, file, black, k);
	}

	if (isPromoting) 
		gui.drawPromotion();

	//if (Checkmate)
	//	cout << "checkmate" << endl;
	if (Stalemate)
		cout << "stalemate" << endl;
	if (Draw)
		cout << "draw" << endl;

	glDisable(GL_BLEND);
	glutSwapBuffers();
}

void reshape(int w, int h) {
	screenWidth = w;
	screenHeight = h;


	sqW = w / 8 * 1.75;
	sqH = h / 8 * 1.75;

	glViewport(0, 0, w, h);	// set Viewport dimensions

	ProjectionMatrix = glm::ortho(-w/2.0 - 50, w/2.0 + 50, -h/2.0 - 50, h/2.0 + 50);
}

void idle() {
	glutMouseFunc(mouseCallback);
	glutPostRedisplay();
}

//=User Inputs====||==================||==================||==================||==================>>

void mouseCallback(int button, int state, int x, int y) {
	//moving the origin to the centre of the screen
	x -= screenWidth / 2;
	y -= screenHeight / 2;
	y *= -1;
	
	int width = sqW / 2;
	int height = sqH / 2;

	//moving the origin to the bottom left of the board
	x += width * 4;
	y += height * 4;
	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) return;
	
	if (Checkmate || Stalemate || Draw) return;

	//Get promotion choice----------------------------------------------------------------------------------------
	if (isPromoting) {
		for (int i = 0; i < 4; i++) 
		{
			float squareX = ((i + 26) % 8) * width + width / 2;
			float squareY = ((i + 26) / 8) * height + height;
			if (squareX - width / 2 < x && x < squareX + width / 2) {
				if (squareY - height < y && y < squareY + height) 
					promotionChoice = static_cast<Promotion>(i);
			}
		}
	}
	//check every square------------------------------------------------------------------------------------------
	for (int sq = 0; sq < 64; sq++) 
	{
		float squareX = (sq % 8) * width + width / 2;
		float squareY = (sq / 8) * height + height / 2;
		if (squareX - width / 2 < x && x < squareX + width / 2) {
			if (squareY - height / 2 < y && y < squareY + height / 2) {
				if (!isPromoting) {
					targetSq = null;
					if (Wpieces.test(sq) && turn == white || Bpieces.test(sq) && turn == black) {
						if (originSq != static_cast<enumSquare>(sq)) { 
							originSq = static_cast<enumSquare>(sq); 
							attackBB = mg.getLegalMoves(originSq, getPieceColour(originSq), getPieceType(originSq))
								   | mg.getLegalCaptures(originSq, getPieceColour(originSq), getPieceType(originSq));
						}
						else
							originSq = null;
						return;
					}
					if (originSq == null) return; 
					
					targetSq = static_cast<enumSquare>(sq);

					if (getPieceType(originSq) == p) {
						if ((56 <= targetSq && getPieceColour(originSq) == white) || (targetSq < 8 && getPieceColour(originSq) == black)) {
							isPromoting = true;
							return;
						}
					}
				}
				else {
					if(promotionChoice == toNone){
						originSq = null;
						targetSq = null;
						isPromoting = false;
						promotionChoice = toNone;
						return;
					}
				}

				if (originSq == null || targetSq == null) return;

				if (!attackBB.test(targetSq)) return;
				
				makeMove(originSq, targetSq, getPieceColour(originSq), getPieceType(originSq), promotionChoice);
				Wcheck = isCheck(white); Bcheck = isCheck(black);
				Checkmate = isCheckMate(turn);
				Stalemate = isStaleMate(turn);
				Draw = isDraw();

				originSq = null; targetSq = null;
				isPromoting = false;

				if (!(Checkmate||Stalemate||Draw)) {
					evaluator.reset();
					evaluator.minimax(4, -INFINITY, INFINITY);
					cout << evaluator.getBestMove().getMoveCode() << endl;
					Move m = evaluator.getBestMove();
					makeMove(m.getOrigin(), m.getDestination(), m.getColour(), m.getType(), m.getPromotion());
					Wcheck = isCheck(white); Bcheck = isCheck(black);
					Checkmate = isCheckMate(turn);
					Stalemate = isStaleMate(turn);
					Draw = isDraw();
				}
			}
		}
	}
}

//===================================<<---------------------->>===================================//
//===================================<< END OPENGL FUNCTIONS >>===================================//
//===================================<<---------------------->>===================================//


void makeMove(enumSquare origin, enumSquare target, Colour c, Type t, Promotion promoteTo) {
	
	Move m = Move(origin, target, c, t);
	
	Type tt = getPieceType(target);
	if (tt != None) {
		HMcounter = -1;
		m.setTakenType(tt);
	}
	if (promoteTo != toNone)
		m.setPromotion(promoteTo);

	pCastlingRights = castlingRights;
	pEPTargets = EPTargets;

	//empty the square to be taken in all bitboards
	switch (tt) {
	case p:
		c == white ? Bp.set(target, 0) : Wp.set(target, 0);
		break;
	case n:
		c == white ? Bn.set(target, 0) : Wn.set(target, 0);
		break;
	case b:
		c == white ? Bb.set(target, 0) : Wb.set(target, 0);
		break;
	case r:
		c == white ? Br.set(target, 0) : Wr.set(target, 0);
		break;
	case q:
		c == white ? Bq.set(target, 0) : Wq.set(target, 0);
		break;
	case k:
		c == white ? Bk.set(target, 0) : Wk.set(target, 0);
		break;
	}

	switch (t) {
	case p:
		HMcounter = -1;
		if (c == white) {
			Wp.set(origin, 0); Wp.set(target, 1);
			//find out if the move was an en passent
			if (EPTargets.test(target)) {
				Bp.set(target - 8, 0);
				m.isEP = true;
			}
			EPTargets.reset();
			//if the move is a two square move, set the en passent target
			if (target - origin == 16)
				EPTargets.set(origin + 8, 1);

			if (56 <= target && target < 64) {
				Wp.set(target, 0);
				promoteTo == toN ? Wn.set(target, 1) :
					promoteTo == toB ? Wb.set(target, 1) :
					promoteTo == toR ? Wr.set(target, 1) :
					promoteTo == toQ ? Wq.set(target, 1) :
					__noop;
			}
		}
		else {
			Bp.set(origin, 0); Bp.set(target, 1);
			//find out if the move was an en passent
			if (EPTargets.test(target)) {
				Wp.set(target + 8, 0);
				m.isEP = true;
			}
			EPTargets.reset();
			//if the move is a two square move, set the en passent target
			if (origin - target == 16)
				EPTargets.set(origin - 8, 1);

			if (0 <= target && target < 8) {
				Bp.set(target, 0);
				promoteTo == toN ? Bn.set(target, 1) :
					promoteTo == toB ? Bb.set(target, 1) :
					promoteTo == toR ? Br.set(target, 1) :
					promoteTo == toQ ? Bq.set(target, 1) :
					__noop;
			}
		}	
		break;
	case n:
		if (c == white) {
			Wn.set(origin, 0); 
			Wn.set(target, 1);
		}
		else {
			Bn.set(origin, 0); 
			Bn.set(target, 1);
		}
		break;
	case b:
		if (c == white) {
			Wb.set(origin, 0); 
			Wb.set(target, 1);
		}
		else {
			Bb.set(origin, 0); 
			Bb.set(target, 1);
		}
		break;
	case r:
		if (c == white) {
			Wr.set(origin, 0); 
			Wr.set(target, 1);
		}
		else {
			Br.set(origin, 0);
			Br.set(target, 1);
		}
		break;
	case q:
		if (c == white) {
			Wq.set(origin, 0);
			Wq.set(target, 1);
		}
		else {
			Bq.set(origin, 0);
			Bq.set(target, 1);
		}
		break;
	case k:
		if (c == white) {
			Wk.set(origin, 0); Wk.set(target, 1);
			
			if (origin - target == 2) {
				Wr.set(0, 0); Wr.set(3, 1);
				m.isCastle = true;
			}
			else if (target - origin == 2) {
				Wr.set(7, 0); Wr.set(5, 1);
				m.isCastle = true;
			}
		}
		else {
			Bk.set(origin, 0); Bk.set(target, 1);
			if (origin - target == 2) {
				Br.set(56, 0); Br.set(59, 1);
				m.isCastle = true;
			}
			else if (target - origin == 2) {
				Br.set(61, 1); Br.set(63, 0);
				m.isCastle = true;
			}
		}
		break;
	}
	Wpieces = Wp | Wn | Wb | Wr | Wq | Wk;
	Bpieces = Bp | Bn | Bb | Br | Bq | Bk;
	Occupied = Wpieces | Bpieces;

	if (origin == 4) {
		castlingRights.set(0, 0);
		castlingRights.set(1, 0);
	}
	else if (origin == 60) {
		castlingRights.set(2, 0);
		castlingRights.set(3, 0);
	}

	if (castlingRights.test(0) && (origin ==  0 || target ==  0)) castlingRights.set(0, 0);
	if (castlingRights.test(0) && (origin ==  7 || target ==  7)) castlingRights.set(1, 0);
	if (castlingRights.test(0) && (origin == 56 || target == 56)) castlingRights.set(2, 0);
	if (castlingRights.test(0) && (origin == 63 || target == 63)) castlingRights.set(3, 0);

	if (t != p) EPTargets.reset();

	turn = turn == white ? black : white;
	HMcounter++;
	if (c == black)
		FMcounter++;

	movesMade.push_back(m);
}


void unmakeMove() {
	if (movesMade.empty()) return;
	Move move = movesMade.back();

	Colour c = move.getColour();
	Type t = move.getType();
	Type tt = move.getTakenType();
	bool isTake = move.isTake();
	enumSquare o = move.getOrigin();
	enumSquare d = move.getDestination();
	Promotion promoteTo = move.getPromotion();

	castlingRights = pCastlingRights;
	EPTargets = pEPTargets;

	if (t == p) {
		c == white ? Wp.set(d, 0) : Bp.set(d, 0);
		c == white ? Wp.set(o, 1) : Bp.set(o, 1);
	}
	else if (t == r) {
		c == white ? Wr.set(d, 0) : Br.set(d, 0);
		c == white ? Wr.set(o, 1) : Br.set(o, 1);
	}
	else if (t == n){
		c == white ? Wn.set(d, 0) : Bn.set(d, 0);
		c == white ? Wn.set(o, 1) : Bn.set(o, 1);
	}
	else if (t == b){
		c == white ? Wb.set(d, 0) : Bb.set(d, 0);
		c == white ? Wb.set(o, 1) : Bb.set(o, 1);
	}
	else if (t == q){
		c == white ? Wq.set(d, 0) : Bq.set(d, 0);
		c == white ? Wq.set(o, 1) : Bq.set(o, 1);
	}
	else if (t == k){
		c == white ? Wk.set(d, 0) : Bk.set(d, 0);
		c == white ? Wk.set(o, 1) : Bk.set(o, 1);
	}


	//if take, put the piece back
	if (isTake) {
		if (tt == p)
			c == white ? Bp.set(d, 1) : Wp.set(d, 1);
		else if (tt == r)
			c == white ? Br.set(d, 1) : Wr.set(d, 1);
		else if (tt == n)
			c == white ? Bn.set(d, 1) : Wn.set(d, 1);
		else if (tt == b)
			c == white ? Bb.set(d, 1) : Wb.set(d, 1);
		else if (tt == q)
			c == white ? Bq.set(d, 1) : Wq.set(d, 1);
		else if (tt == k)
			c == white ? Bk.set(d, 1) : Wk.set(d, 1);
	}
	else if (move.isCastle) {
		if (d == 2) { //white queen side castle
			Wr.set(3, 0);
			Wr.set(0, 1);
		}
		else if (d == 6) { //white king side castle
			Wr.set(5, 0);
			Wr.set(7, 1);
		}
		else if (d == 58) { //black queen side castle
			Br.set(59, 0);
			Br.set(56, 1);
		}
		else if (d == 62) { //black king side castle
			Br.set(61, 0);
			Br.set(63, 1);
		}
	}


	//if it was a diagonal pawn move but wasn't a take, then it was an en passent
	if (move.isEP) {
		int dir = o - d;
		if (dir == -9 || dir == -7) Bp.set(d - 8, 1);
		else if (dir == 9 || dir == 7) Wp.set(d + 8, 1);
	}

	if (promoteTo != toNone) {
		switch (promoteTo) {
		case toN:
			c == white ? Wn.set(d, 0) : Bn.set(d, 0);
		case toB:
			c == white ? Wb.set(d, 0) : Bb.set(d, 0);
		case toR:
			c == white ? Wr.set(d, 0) : Br.set(d, 0);
		case toQ:
			c == white ? Wq.set(d, 0) : Bq.set(d, 0);
		}
	}

	//update the occupied bitboard
	Wpieces = Wp | Wr | Wn | Wb | Wq | Wk;
	Bpieces = Bp | Br | Bn | Bb | Bq | Bk;
	Occupied = Wpieces | Bpieces;

	HMcounter--;
	if (c == black)
		FMcounter--;

	turn = turn == white ? black : white;
	//movesMade.erase(std::remove(movesMade.begin(), movesMade.end(), move), movesMade.end());
	movesMade.pop_back();
}

//=Misc===========||==================||==================||==================||==================>>

//returns the type of the piece in the entered square
Type getPieceType(int sq) {
	if (Wp.test(sq) || Bp.test(sq)) return p;
	if (Wr.test(sq) || Br.test(sq)) return r;
	if (Wn.test(sq) || Bn.test(sq)) return n;
	if (Wb.test(sq) || Bb.test(sq)) return b;
	if (Wq.test(sq) || Bq.test(sq)) return q;
	if (Wk.test(sq) || Bk.test(sq)) return k;
	return None;
}

//returns the colour of the piece in the entered square
Colour getPieceColour(int sq) {
	if (Wpieces.test(sq)) return white;
	if (Bpieces.test(sq)) return black;
	return NA;
}

int getFirstOfRank(int sq) {
	return sq - sq % 8;
}

int getLastOfRank(int sq) {
	return sq + 7 - sq % 8;
}

//=End of Game====||==================||==================||==================||==================>>

bool isCheck(Colour colourInCheck) {
	bitset<64> king = colourInCheck == white ? Wk : Bk;
	const enum dirs { SW = -9, S = -8, SE = -7, W = -1, E = 1, NW = 7, N = 8, NE = 9 };
	//find king position
	int kingPos = 0;
	for (int sq = 0; sq < 64; sq++) {
		if (!king.test(sq)) continue;
		kingPos = sq;
		break;
	}

	bitset<64> opponantBitboard = colourInCheck == white ? Bpieces : Wpieces;
	//north & north-west
	if (kingPos < 56) { // north
		if (opponantBitboard.test(kingPos + N) && getPieceType(kingPos + N) == k)
			return true;
		for (int sq = kingPos + N; sq < 64; sq += N) {
			if (!Occupied.test(sq)) continue;
			if (!opponantBitboard.test(sq)) break;
			if (getPieceType(sq) == r || getPieceType(sq) == q) return true;
			break;
		}
		if (kingPos % 8 > 0) { // north-west
			if (opponantBitboard.test(kingPos + NW) && getPieceType(kingPos + NW) == k)
				return true;
			for (int sq = kingPos + NW; sq < 64; sq += NW) {
				if (sq % 8 > kingPos % 8) break; // left blocker
				if (!Occupied.test(sq)) continue;
				if (!opponantBitboard.test(sq)) break;
				if (getPieceType(sq) == b || getPieceType(sq) == q) return true;
				break;
			}
		}
	}
	//west & south-west
	if (kingPos % 8 > 0) { // west
		if (opponantBitboard.test(kingPos + W) && getPieceType(kingPos + W) == k)
			return true;
		for (int sq = kingPos + W; sq >= getFirstOfRank(kingPos); sq += W) {
			if (!Occupied.test(sq)) continue;
			if (!opponantBitboard.test(sq)) break;
			if (getPieceType(sq) == r || getPieceType(sq) == q) return true;
			break;
		}
		if (kingPos >= 8) { //south-west
			if (opponantBitboard.test(kingPos + SW) && getPieceType(kingPos + SW) == k)
				return true;
			for (int sq = kingPos + SW; sq >= 0; sq += SW) {
				if (sq % 8 > kingPos % 8) break; // left blocker
				if (!Occupied.test(sq)) continue;
				if (!opponantBitboard.test(sq)) break;
				if (getPieceType(sq) == b || getPieceType(sq) == q) return true;
				break;
			}
		}
	}
	//south & south-east
	if (kingPos >= 8) {//south
		if (opponantBitboard.test(kingPos + S) && getPieceType(kingPos + S) == k)
			return true;
		for (int sq = kingPos + S; sq >= 0; sq += S) {
			if (!Occupied.test(sq)) continue;
			if (!opponantBitboard.test(sq)) break;
			if (getPieceType(sq) == r || getPieceType(sq) == q) return true;
			break;
		}
		if (kingPos % 8 < 7) { //south-east
			if (opponantBitboard.test(kingPos + SE) && getPieceType(kingPos + SE) == k)
				return true;
			for (int sq = kingPos + SE; sq >= 0; sq += SE) {
				if (sq % 8 < kingPos % 8) break; // right blocker
				if (!Occupied.test(sq)) continue;
				if (!opponantBitboard.test(sq)) break;
				if (getPieceType(sq) == b || getPieceType(sq) == q) return true;
				break;
			}
		}
	}
	//east & north-east
	if (kingPos % 8 < 7) { //east
		if (opponantBitboard.test(kingPos + E) && getPieceType(kingPos + E) == k)
			return true;
		for (int sq = kingPos + E; sq <= getLastOfRank(kingPos); sq += E) {
			if (!Occupied.test(sq)) continue;
			if (!opponantBitboard.test(sq)) break;
			if (getPieceType(sq) == r || getPieceType(sq) == q) return true;
			break;
		}
		if (kingPos < 56) { //north-east
			if (opponantBitboard.test(kingPos + NE) && getPieceType(kingPos + NE) == k)
				return true;
			for (int sq = kingPos + NE; sq < 64; sq += NE) {
				if (sq % 8 < kingPos % 8) break; // right blocker
				if (!Occupied.test(sq)) continue;
				if (!opponantBitboard.test(sq)) break;
				if (getPieceType(sq) == b || getPieceType(sq) == q) return true;
				break;
			}
		}
	}

	const enum KnightDirs {
		SoSoWe = -17, SoSoEa = -15, SoWeWe = -10, SoEaEa = -6,
		NoWeWe = 6, NoEaEa = 10, NoNoWe = 15, NoNoEa = 17
	};

	//knight attack check
	if (kingPos % 8 > 0) { // not on the a file | 1We
		if (kingPos >= 17) { // not on bottom 2 rows
			if (opponantBitboard.test(kingPos + SoSoWe)) {
				if (getPieceType(kingPos + SoSoWe) == n) return true;
			}
		}
		if (kingPos < 48) { // not on top 2 rows
			if (opponantBitboard.test(kingPos + NoNoWe)) {
				if (getPieceType(kingPos + NoNoWe) == n) return true;
			}
		}
	}
	if (kingPos % 8 > 1) { // not on a or b file | 2We
		if (kingPos >= 8) { // not on bootom row
			if (opponantBitboard.test(kingPos + SoWeWe)) {
				if (getPieceType(kingPos + SoWeWe) == n) return true;
			}
		}
		if (kingPos < 56) { // not on top row
			if (opponantBitboard.test(kingPos + NoWeWe)) {
				if (getPieceType(kingPos + NoWeWe) == n) return true;
			}
		}
	}
	if (kingPos % 8 < 6) { // not on the g or h file | 2Ea
		if (kingPos >= 8) { // not on the bottom row
			if (opponantBitboard.test(kingPos + SoEaEa)) {
				if (getPieceType(kingPos + SoEaEa) == n) return true;
			}
		}
		if (kingPos < 56) { // not on the top row
			if (opponantBitboard.test(kingPos + NoEaEa)) {
				if (getPieceType(kingPos + NoEaEa) == n) return true;
			}
		}
	}
	if (kingPos % 8 < 7) { // 1Ea
		if (kingPos >= 16) { // not on the bottom 2 rows
			if (opponantBitboard.test(kingPos + SoSoEa)) {
				if (getPieceType(kingPos + SoSoEa) == n) return true;
			}
		}
		if (kingPos < 48) { //not on the top 2 rows
			if (opponantBitboard.test(kingPos + NoNoEa)) {
				if (getPieceType(kingPos + NoNoEa) == n) return true;
			}
		}
	}

	//pawn attack check
	if (colourInCheck == white && kingPos < 56) {
		if (kingPos % 8 > 0) {
			if (opponantBitboard.test(kingPos + NW)) {
				if (getPieceType(kingPos + 7) == p) return true;
			}
		}
		if (kingPos % 8 < 7) {
			if (opponantBitboard.test(kingPos + NE)) {
				if (getPieceType(kingPos + 9) == p) return true;
			}
		}
	}
	if (colourInCheck == black && kingPos >= 8) {
		if (kingPos % 8 > 0) {
			if (opponantBitboard.test(kingPos + SW)) {
				if (getPieceType(kingPos - 9) == p) return true;
			}
		}
		if (kingPos % 8 < 7) {
			if (opponantBitboard.test(kingPos + SE)) {
				if (getPieceType(kingPos - 7) == p) return true;
			}
		}
	}

	return false;
}

bool isCheckMate(Colour colourInCheck) {
	if (!isCheck(colourInCheck)) return false;
	
	bitset<64> colourBB = colourInCheck == white ? Wpieces : Bpieces;
	for (int sq = 0; sq < 64; sq++) {
		if (!colourBB.test(sq)) continue;
		bitset<64> attacks = mg.getLegalMoves(static_cast<enumSquare>(sq), colourInCheck, getPieceType(sq))
			| mg.getLegalCaptures(static_cast<enumSquare>(sq), colourInCheck, getPieceType(sq));

		if (attacks.count() != 0) return false;
	}
	return true;
}

bool isStaleMate(Colour colourToMove) {

	if (isCheck(colourToMove)) return false;

	bitset<64> colourBB = colourToMove == white ? Wpieces : Bpieces;


	for (int origin = 0; origin < 64; origin++) {
		if (!colourBB.test(origin)) continue;

		Colour c = colourToMove;
		Type t = getPieceType(origin);

		bitset<64> attacks = mg.getLegalMoves(static_cast<enumSquare>(origin), c, t)
			| mg.getLegalCaptures(static_cast<enumSquare>(origin), c, t);

		if (attacks.none()) continue;
		for (int target = 0; target < 64; target++) {
			if (!attacks.test(target)) continue;
			makeMove(static_cast<enumSquare>(origin), static_cast<enumSquare>(target), c, t);
			if (!isCheck(colourToMove)) {
				unmakeMove();
				return false;
			}
			unmakeMove();
		}
	}
	return true;
}

bool isDraw() {
	//50 move rule
	if (HMcounter >= 100) return true;
	//insufficient material rule 
	if (Occupied.count() <= 2) return true; //only 2 kings left
	if (Wb.count() <= 1 && Bb.count() <= 1)
		if (Wn.count() <= 1 && Bn.count() <= 1)
			if ((Wp | Wr | Wq | Bp | Br | Bq).count() == 0)
				return true;
	//TODO: draw by repetition
	return false;
}

//=Load board=====||==================||==================||==================||==================>>

//takes in a FEN string and updates the bitboards
void loadFromFen(string fen) {
	string fen_info[6] = { "", "", "", "", "", "" };

	//=split the fen into componants===========================================|
	string temp = "";
	int counter = 0;
	for (int i = 0; i < fen.length(); i++) {
		if (fen[i] != ' ')
			temp += fen[i];
		else {
			fen_info[counter] = temp;
			temp = "";
			counter++;
		}
	}

	//=reverse the fen string==================================================|
	string fenRow[8];
	counter = 0;
	for (int i = 0; i < fen_info[0].length(); i++) {
		if (fen_info[0][i] == '/') {
			counter++;
			continue;
		}
		fenRow[counter].push_back(fen[i]);
	}
	fen_info[0] = "";
	for (int i = 7; i >= 0; i--)
		fen_info[0].append(fenRow[i]);

	//=Piece Positions=======================================================0=|
	Wp.reset();	Wr.reset(); Wn.reset();
	Wb.reset(); Wq.reset();	Wk.reset();

	Bp.reset();	Br.reset(); Bn.reset();
	Bb.reset(); Bq.reset();	Bk.reset();


	int pos = 0;
	for (char c : fen_info[0]) {
		if (isdigit(c)) {
			pos += int(c) - 48;
			continue;
		}
		c == 'P' ? Wp.set(pos, 1) :
			c == 'p' ? Bp.set(pos, 1) :
			c == 'R' ? Wr.set(pos, 1) :
			c == 'r' ? Br.set(pos, 1) :
			c == 'N' ? Wn.set(pos, 1) :
			c == 'n' ? Bn.set(pos, 1) :
			c == 'B' ? Wb.set(pos, 1) :
			c == 'b' ? Bb.set(pos, 1) :
			c == 'Q' ? Wq.set(pos, 1) :
			c == 'q' ? Bq.set(pos, 1) :
			c == 'K' ? Wk.set(pos, 1) :
			c == 'k' ? Bk.set(pos, 1) : __noop;
		pos++;
	}

	Wpieces = Wp | Wr | Wn | Wb | Wq | Wk;
	Bpieces = Bp | Br | Bn | Bb | Bq | Bk;

	Occupied = Wpieces | Bpieces;

	//=Active Colour=========================================================1=|
	if (fen_info[1] == "b")
		turn = black;
	else
		turn = white;

	//=Castling Rights=======================================================2=|
	castlingRights.reset();
	for (char c : fen_info[2]) {
		c == 'K' ? castlingRights.set(0, 1) :
			c == 'Q' ? castlingRights.set(1, 1) :
			c == 'k' ? castlingRights.set(2, 1) :
			c == 'q' ? castlingRights.set(3, 1) : __noop;
	}

	//=Possible En Passant Targets===========================================3=|
	EPTargets.reset();
	if ((int)fen_info[3][0] != 45) {
		int pos = (int)fen_info[3][0] - 97 + ((int)fen_info[3][1] - 49) * 8;
		EPTargets.set(pos, 1);
	}

	//=Halfmove Clock========================================================4=|
	if ((int)fen_info[4][0] != 45)
		HMcounter = stoi(fen_info[4]);

	//=Fullmove Number=======================================================5=|
	if ((int)fen_info[5][0] != 45)
		FMcounter = stoi(fen_info[5]);
}

//=======================================<<--------------->>======================================//
//=======================================<< MAIN FUNCTION >>======================================//
//=======================================<<--------------->>======================================//

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Chess Bot");

	glutReshapeFunc(reshape);
	
	//This initialises glew - it must be called after the window is created.
	GLenum err = glewInit();
	if (GLEW_OK != err)
		std::cout << " GLEW ERROR" << std::endl;

	pA.setPawnAttacks();
	nA.setKnightAttacks();
	sA.setRayAttacks();
	kA.setKingAttacks();

	//loadFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
	//loadFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");
	loadFromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ");	
	//loadFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1 ");	
	//loadFromFen("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
	//loadFromFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ");
	//loadFromFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");


	Tester tester;
	//perft tests	
	//for (int i = 1; i <= 5; i++) {
	//	auto start = chrono::high_resolution_clock::now();
	//	int result = tester.perft(i);
	//	auto end = chrono::high_resolution_clock::now();
	//	cout << "test " << i << ": " << result << endl;
	//	cout << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000. << endl << endl;
	//}

	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}
