#include<iostream>
#include<string>
#include<bitset>
#include<vector>
#include<chrono>

//--- OpenGL ---
#include "GL\glew.h"
#include "GL\wglew.h"
#pragma comment(lib, "glew32.lib")
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "GL\freeglut.h"
//--------------

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
int moveCount;

bitset<64> attackBB(0);

bool Checkmate = false, Stalemate = false;
bool Wcheck = false, Bcheck = false;
bool isPromoting = false;

Promotion promotionChoice = toNone;

//-------------------------------------------------------------------------------------------------
void mouseCallback(int button, int state, int x, int y);
void drawPiece(float x, float y, Colour c, Type t);
void drawBoard();
void drawSelected(int rank, int file);
void drawAttacks(bitset<64>);
void drawPromotion();

void makeMove(enumSquare origin, enumSquare target, Colour c, Type t, Promotion promoteTo);
void unmakeMove();
bitset<64> getLegalMoves(enumSquare sq, Colour c, Type t);
bitset<64> getLegalCaptures(enumSquare sq, Colour c, Type t);

bool isCheck(Colour colourInCheck);
bool isCheckMate(Colour colourInCheck);
bool isStaleMate(Colour colourToMove);

Colour getPieceColour(int sq);
Type getPieceType(int sq);
int getFirstOfRank(int sq);
int getlastOfRank(int sq);

bitset<64> getLegalMoves(enumSquare sq, Colour c, Type t);
bitset<64> getLegalCaptures(enumSquare sq, Colour c, Type t);
vector<Move> getAllMoves(bool capturesOnly);

bool isCheck(Colour colourInCheck);
bool isCheckMate(Colour colourInCheck);
bool isStaleMate(Colour colourToMove);

void loadFromFen(string fen);

#include "Eval.h"
Evaluator evaluator;
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

	drawBoard();

	if (originSq != null) {
		drawSelected(originSq / 8, originSq % 8);
		drawAttacks(attackBB);
	}

	for (int sq = 0; sq < 64; sq++) {
		if (!Occupied.test(sq)) continue;

		int rank = sq / 8;
		int file = sq % 8;

		if      (Wp.test(sq)) drawPiece(rank, file, white, p);
		else if (Bp.test(sq)) drawPiece(rank, file, black, p);
		else if (Wr.test(sq)) drawPiece(rank, file, white, r);
		else if (Br.test(sq)) drawPiece(rank, file, black, r);
		else if (Wn.test(sq)) drawPiece(rank, file, white, n);
		else if (Bn.test(sq)) drawPiece(rank, file, black, n);
		else if (Wb.test(sq)) drawPiece(rank, file, white, b);
		else if (Bb.test(sq)) drawPiece(rank, file, black, b);
		else if (Wq.test(sq)) drawPiece(rank, file, white, q);
		else if (Bq.test(sq)) drawPiece(rank, file, black, q);
		else if (Wk.test(sq)) drawPiece(rank, file, white, k);
		else if (Bk.test(sq)) drawPiece(rank, file, black, k);
	}

	if (isPromoting) 
		drawPromotion();

	if (Checkmate)
		cout << "checkmate" << endl;
	if (Stalemate)
		cout << "stalemate" << endl;

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
	
	if (Checkmate || Stalemate) return;

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
							attackBB = getLegalMoves(originSq, getPieceColour(originSq), getPieceType(originSq))
								| getLegalCaptures(originSq, getPieceColour(originSq), getPieceType(originSq));
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

				originSq = null;
				targetSq = null;
				isPromoting = false;

				moveCount++;

				evaluator.clear();
				evaluator.minimax(4, -INFINITY, INFINITY);
				cout << evaluator.getBestMove().getMoveCode() << endl;
			}
		}
	}
}

//===================================<<---------------------->>===================================//
//===================================<< END OPENGL FUNCTIONS >>===================================//
//===================================<<---------------------->>===================================//

void drawLine(float x1, float y1, float x2, float y2, Colour c) {
	glBegin(GL_LINES);
	
	c == white ? glColor3f(1.0, 1.0, 1.0) :
	c == black ? glColor3f(0.0, 0.0, 0.0) : glColor3f(0.75, 0.0, 0.0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawPiece(float rank, float file, Colour c, Type t) {
	float w = (sqW * 0.5) / double(screenWidth);
	float h = (sqH * 0.5) / double(screenHeight);

	float x = file * sqW / double(screenWidth) - 0.762;
	float y = rank * sqH / double(screenHeight) - 0.762;
	float ratio = 0.85;
	switch (t) {
	case p:
		drawLine(x, y - h / 3., x, y + h / 3., c);
		break;
	case n:
		drawLine(x, y - h / 2., x, y + h / 2., c);
		drawLine(x - w / 2., y, x + w / 2., y, c);

		drawLine(x - w / 2., y + h / 4., x - w / 2., y - h / 4., c);
		drawLine(x + w / 2., y + h / 4., x + w / 2., y - h / 4., c);

		drawLine(x - w / 4., y + h / 2., x + w / 4., y + h / 2., c);
		drawLine(x - w / 4., y - h / 2., x + w / 4., y - h / 2., c);
		break;
	case b:
		drawLine(x - w / 2. * ratio, y + h / 2. * ratio, x + w / 2. * ratio, y - h / 2. * ratio, c);
		drawLine(x - w / 2. * ratio, y - h / 2. * ratio, x + w / 2. * ratio, y + h / 2. * ratio, c);
		break;
	case r:
		drawLine(x - w / 2., y, x + w / 2., y, c);
		drawLine(x, y - h / 2., x, y + h / 2., c);
		break;
	case q:
		drawLine(x - w / 2. * ratio, y + h / 2. * ratio, x + w / 2. * ratio, y - h / 2. * ratio, c);
		drawLine(x - w / 2. * ratio, y - h / 2. * ratio, x + w / 2. * ratio, y + h / 2. * ratio, c);

		drawLine(x - w / 2., y, x + w / 2., y, c);
		drawLine(x, y - h / 2., x, y + h / 2., c);
		break;
	case k:
		if (Wcheck && c == white || Bcheck && c == black) c = NA;
		drawLine(x - w / 2, y - h / 2, x + w / 2, y - h / 2, c);
		drawLine(x + w / 2, y - h / 2, x + w / 2, y + h / 2, c);
		drawLine(x + w / 2, y + h / 2, x - w / 2, y + h / 2, c);
		drawLine(x - w / 2, y + h / 2, x - w / 2, y - h / 2, c);
		break;
	}
}

void drawBoard() {
	double w = sqW / double(screenWidth);
	double h = sqH / double(screenHeight);
	for (int file = 0; file < 8; file++){
		for (int rank = 0; rank < 8; rank++) {
			double x = rank * w - 0.762;
			double y = file * h - 0.762;
			glBegin(GL_POLYGON);
			(file + rank) % 2 == 1 ?
				//glColor3f(0.80f, 0.65f, 0.50f):  
				//glColor3f(0.50f, 0.30f, 0.20f);  
				glColor3f(0.40f, 0.75f, 1.00f) :
				glColor3f(0.10f, 0.50f, 0.70f);
			glVertex2d(x - w/2., y - h/2.);
			glVertex2d(x - w/2., y + h/2.);
			glVertex2d(x + w/2., y + h/2.);
			glVertex2d(x + w/2., y - h/2.);
			glEnd();
		}
	}
}

void drawSelected(int rank, int file) {
	double w = sqW / double(screenWidth);
	double h = sqH / double(screenHeight);
	double x = file * w - 0.762;
	double y = rank * h - 0.762;
	glBegin(GL_POLYGON);
	glColor3f(0.5f, 0.25f, 0.75f);
	glVertex2d(x - w / 2., y - h / 2.);
	glVertex2d(x - w / 2., y + h / 2.);
	glVertex2d(x + w / 2., y + h / 2.);
	glVertex2d(x + w / 2., y - h / 2.);
	glEnd();
}

void drawAttacks(bitset<64> attacks) {
	for (int i = 0; i < 64; i++) {
		if (!attacks.test(i)) continue;
		int rank = i / 8;
		int file = i % 8;

		float w = (sqW/2) / double(screenWidth);
		float h = (sqH/2) / double(screenHeight);

		double cx = file * sqW / double(screenWidth) - 0.762;
		double cy = rank * sqH / double(screenHeight) - 0.762;
		
		glBegin(GL_LINE_LOOP);
		
		getPieceType(i) == None ? glColor3f(0.0f, 0.75f, 0.0f) : glColor3f(0.75f, 0.0f, 0.0f);
		for (int j = 0; j < 20; j++) {
			float theta = 2.0f * 3.1415926f * float(j) / float(20);//get the current angle 
			float x = w/2 * cosf(theta);//calculate the x component 
			float y = h/2 * sinf(theta);//calculate the y component 
			glVertex2f(x + cx, y + cy);//output vertex 
		}
		glEnd();
	}
}

void drawPromotion() {
	double w = sqW / double(screenWidth);
	double h = sqH / double(screenHeight);

	enumSquare squares[8] = { c4, d4, e4, f4, c5, d5, e5, f5 };
	turn == white ? glColor3f(0.0, 0.0, 0.0) : glColor3f(1.0, 1.0, 1.0);
	for (enumSquare sq : squares) {
		double x = sq % 8 * w - 0.762;
		double y = sq / 8 * h - 0.762;

		glBegin(GL_POLYGON);	
		glVertex2d(x - w / 2., y - h / 2.);
		glVertex2d(x - w / 2., y + h / 2.);
		glVertex2d(x + w / 2., y + h / 2.);
		glVertex2d(x + w / 2., y - h / 2.);
		glEnd();
	}

	drawPiece(3.5, 2, turn, n);
	drawPiece(3.5, 3, turn, b);
	drawPiece(3.5, 4, turn, r);
	drawPiece(3.5, 5, turn, q);
	for (int i = 3; i <= 5; i++) {
		double x = i * w - 0.762 - w / 2.;
		double y1 = 3 * h - 0.762 - h / 2.;
		double y2 = 5 * h - 0.762 - h / 2.;
		drawLine(x, y1, x, y2, turn);
	}

}

//================||==================||==================||==================||==================>>

void makeMove(enumSquare origin, enumSquare target, Colour c, Type t, Promotion promoteTo = toNone) {
	
	Move m = Move(origin, target, c, t);
	
	Type tt = getPieceType(target);
	if (tt != None)
		m.setTakenType(tt);
	if (promoteTo != toNone)
		m.setPromotion(promoteTo);

	pCastlingRights = castlingRights;
	pEPTargets = EPTargets;

	//empty the square to be taken in all bitboards
	Bp.set(target, 0); Bn.set(target, 0); Bb.set(target, 0);
	Br.set(target, 0); Bq.set(target, 0); Bk.set(target, 0);

	Wp.set(target, 0); Wn.set(target, 0); Wb.set(target, 0);
	Wr.set(target, 0); Wq.set(target, 0); Wk.set(target, 0);

	switch (t) {
	case p:
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

	if (origin ==  0 || target ==  0) castlingRights.set(0, 0);
	if (origin ==  7 || target ==  7) castlingRights.set(1, 0);
	if (origin == 56 || target == 56) castlingRights.set(2, 0);
	if (origin == 63 || target == 63) castlingRights.set(3, 0);

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

	castlingRights = pCastlingRights;
	EPTargets = pEPTargets;

	//set the destination to 0 in all the bitboards
	Wp.set(move.getDestination(), 0); Bp.set(move.getDestination(), 0);
	Wr.set(move.getDestination(), 0); Br.set(move.getDestination(), 0);
	Wn.set(move.getDestination(), 0); Bn.set(move.getDestination(), 0);
	Wb.set(move.getDestination(), 0); Bb.set(move.getDestination(), 0);
	Wq.set(move.getDestination(), 0); Bq.set(move.getDestination(), 0);
	Wk.set(move.getDestination(), 0); Bk.set(move.getDestination(), 0);

	//reset the origin back to what it was
	if (move.getType() == p)
		move.getColour() == white ? Wp.set(move.getOrigin(), 1) : Bp.set(move.getOrigin(), 1);
	if (move.getType() == r)
		move.getColour() == white ? Wr.set(move.getOrigin(), 1) : Br.set(move.getOrigin(), 1);
	if (move.getType() == n)
		move.getColour() == white ? Wn.set(move.getOrigin(), 1) : Bn.set(move.getOrigin(), 1);
	if (move.getType() == b)
		move.getColour() == white ? Wb.set(move.getOrigin(), 1) : Bb.set(move.getOrigin(), 1);
	if (move.getType() == q)
		move.getColour() == white ? Wq.set(move.getOrigin(), 1) : Bq.set(move.getOrigin(), 1);
	if (move.getType() == k)
		move.getColour() == white ? Wk.set(move.getOrigin(), 1) : Bk.set(move.getOrigin(), 1);

	//if take, put the piece back
	if (move.isTake()) {
		if (move.getTakenType() == p)
			move.getColour() == white ? Bp.set(move.getDestination(), 1) : Wp.set(move.getDestination(), 1);
		else if (move.getTakenType() == r)
			move.getColour() == white ? Br.set(move.getDestination(), 1) : Wr.set(move.getDestination(), 1);
		else if (move.getTakenType() == n)
			move.getColour() == white ? Bn.set(move.getDestination(), 1) : Wn.set(move.getDestination(), 1);
		else if (move.getTakenType() == b)
			move.getColour() == white ? Bb.set(move.getDestination(), 1) : Wb.set(move.getDestination(), 1);
		else if (move.getTakenType() == q)
			move.getColour() == white ? Bq.set(move.getDestination(), 1) : Wq.set(move.getDestination(), 1);
		else if (move.getTakenType() == k)
			move.getColour() == white ? Bk.set(move.getDestination(), 1) : Wk.set(move.getDestination(), 1);

	}
	else if (move.isCastle) {
		if (move.getDestination() == 2) { //white queen side castle
			Wr.set(3, 0);
			Wr.set(0, 1);
		}
		else if (move.getDestination() == 6) { //white king side castle
			Wr.set(5, 0);
			Wr.set(7, 1);
		}
		else if (move.getDestination() == 58) { //black queen side castle
			Br.set(59, 0);
			Br.set(56, 1);
		}
		else if (move.getDestination() == 62) { //black king side castle
			Br.set(61, 0);
			Br.set(63, 1);
		}
	}


	//if it was a diagonal pawn move but wasn't a take, then it was an en passent
	if (move.isEP) {
		int dir = move.getOrigin() - move.getDestination();
		if (dir == -9 || dir == -7) Bp.set(move.getDestination() - 8, 1);
		else if (dir == 9 || dir == 7) Wp.set(move.getDestination() + 8, 1);
	}

	//update the occupied bitboard
	Wpieces = Wp | Wr | Wn | Wb | Wq | Wk;
	Bpieces = Bp | Br | Bn | Bb | Bq | Bk;
	Occupied = Wpieces | Bpieces;

	HMcounter--;
	if (move.getColour() == black)
		FMcounter--;

	turn = turn == white ? black : white;
	movesMade.pop_back();
}

//================||==================||==================||==================||==================>>

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

//================||==================||==================||==================||==================>>

bitset<64> getLegalMoves(enumSquare sq, Colour c, Type t) {
	bitset<64> moves(0),  mask(0);
	switch (t) {
	case p:
		if (c == white) {
			if (Occupied.test(sq + 8)) break;
			makeMove(sq, static_cast<enumSquare>(sq + 8), c, t);
			if(!isCheck(c)) moves.set(sq + 8, 1);
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
			if (!isCheck(c)) moves.set(sq - 8, 1);
			unmakeMove();
			if (sq >= 48 && !Occupied.test(sq - 8) && !Occupied.test(sq - 16)) {
				makeMove(sq, static_cast<enumSquare>(sq - 16), c, t);
				if (!isCheck(c)) moves.set(sq - 16, 1);
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

bitset<64> getLegalCaptures(enumSquare sq, Colour c, Type t) {
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

vector<Move> getAllMoves(bool capturesOnly) {
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
					moves.push_back(move);
				}
			}
			else {
				Move move = Move(static_cast<enumSquare>(sq), static_cast<enumSquare>(target), c, t);
				if ((~bitboard & Occupied).test(target))
					move.setTakenType(getPieceType(target));
				moves.push_back(move);
			}
		}
	}

	return moves;
}
//================||==================||==================||==================||==================>>

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
		bitset<64> attacks = getLegalMoves(static_cast<enumSquare>(sq), colourInCheck, getPieceType(sq))
			| getLegalCaptures(static_cast<enumSquare>(sq), colourInCheck, getPieceType(sq));

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

		bitset<64> attacks = getLegalMoves(static_cast<enumSquare>(origin), c, t)
			| getLegalCaptures(static_cast<enumSquare>(origin), c, t);

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

	Wpieces = Wp| Wr| Wn| Wb| Wq| Wk;
	Bpieces = Bp| Br| Bn| Bb| Bq| Bk;

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

#include "Testing.h"


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
	
	evaluator.minimax(4, -INFINITY, INFINITY);
	cout << evaluator.getBestMove().getMoveCode() << endl;
	
	//perft tests	
	//for (int i = 1; i <= 4; i++) {
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
