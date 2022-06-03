#include<iostream>
#include<string>
#include<bitset>

//--- OpenGL ---
#include "GL\glew.h"
#include "GL\wglew.h"
#pragma comment(lib, "glew32.lib")
//--------------

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "GL\freeglut.h"

#include "Bitboards.h"


const enum Colour { white, black, NA };
const enum Type {p, r, n, b, q, k, None};

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

#include "LookupBitBoards/pawn.h"
#include "LookupBitBoards/knight.h"
#include "LookupBitBoards/king.h"
#include "LookupBitBoards/slidingPieces.h"

int screenWidth = 900, screenHeight = 900;
int sqW, sqH;
glm::mat4 ViewMatrix;					// matrix for the modelling and viewing
glm::mat4 ProjectionMatrix;			// matrix for the orthographic projection

int nbPiecesOnBoard = 0;
Colour turn;

int HMcounter = 0, FMcounter = 0;

enumSquare originSq = null;

pawnAttacks pA;
knightAttacks nA;
slidingAttacks sA;
kingAttacks kA;


//-------------------------------------------------------------------------------------------------
void mouseCallback(int button, int state, int x, int y);
void drawPiece(int x, int y, Colour c, Type t);
void drawBoard();
void drawSelected(int rank, int file);
void drawAttacks(enumSquare sq);

void makeMove(enumSquare origin, enumSquare target, Colour c, Type t);
bitset<64> getLegalMoves(enumSquare sq, Colour c, Type t);
bitset<64> getLegalCaptures(enumSquare sq, Colour c, Type t);

Colour getPieceColour(int pos);
Type getPieceType(int pos);

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

	glm::mat4 ModelViewMatrix;

	drawBoard();

	if (originSq != null) {
		drawSelected(originSq / 8, originSq % 8);
		drawAttacks(originSq);
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

	for (int sq = 0; sq < 64; sq++) {
		int squareX = (sq % 8) * width + width / 2;
		int squareY = (sq / 8) * height + height / 2;
		if (squareX - width / 2 < x && x < squareX + width / 2) {
			if (squareY - height / 2 < y && y < squareY + height / 2) {
				if(Wpieces.test(sq) && turn == white || Bpieces.test(sq) && turn == black) {
					if (originSq != static_cast<enumSquare>(sq)) 
						originSq = static_cast<enumSquare>(sq);
					else 
						originSq = null;
					return;
				}

				if (originSq == null) return;
				
				int rank = originSq / 8;
				int file = originSq % 8;

				enumSquare targetSq = static_cast<enumSquare>(sq);
				Colour targetColour = getPieceColour(targetSq);

				Colour originColour = getPieceColour(originSq);
				Type originType = getPieceType(originSq);

				bitset<64> moves = getLegalMoves(originSq, originColour, originType);
				bitset<64> captures = getLegalCaptures(originSq, originColour, originType);
				
				if (!(moves|captures).test(targetSq)) return;

				if (captures.test(targetSq)) nbPiecesOnBoard--;
				
				makeMove(originSq, targetSq, originColour, originType);

				originSq = null;
				turn == white ? turn = black : turn = white;
								
			}
		}
	}
}

//===================================<<---------------------->>===================================//
//===================================<< END OPENGL FUNCTIONS >>===================================//
//===================================<<---------------------->>===================================//


void drawLine(float x1, float y1, float x2, float y2, Colour c) {
	glBegin(GL_LINES);
	
	c == white ?
		glColor3f(1.0, 1.0, 1.0) :
		glColor3f(0.0, 0.0, 0.0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawPiece(int rank, int file, Colour c, Type t) {
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
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2d(x - w / 2., y - h / 2.);
	glVertex2d(x - w / 2., y + h / 2.);
	glVertex2d(x + w / 2., y + h / 2.);
	glVertex2d(x + w / 2., y - h / 2.);
	glEnd();
}

void drawAttacks(enumSquare sq) {
	bitset<64> moves = getLegalMoves(sq, getPieceColour(sq), getPieceType(sq));
	bitset<64> captures = getLegalCaptures(sq, getPieceColour(sq), getPieceType(sq));
	for (int i = 0; i < 64; i++) {
		if (!(moves|captures).test(i)) continue;
		int rank = i / 8;
		int file = i % 8;

		float w = (sqW/2) / double(screenWidth);
		float h = (sqH/2) / double(screenHeight);

		double cx = file * sqW / double(screenWidth) - 0.762;
		double cy = rank * sqH / double(screenHeight) - 0.762;
		
		glBegin(GL_LINE_LOOP);
		moves.test(i) ? glColor3f(0.0f, 1.0f, 0.0f) : glColor3f(1.0f, 0.0f, 0.0f);
		for (int j = 0; j < 20; j++) {
			float theta = 2.0f * 3.1415926f * float(j) / float(20);//get the current angle 
			float x = w/2 * cosf(theta);//calculate the x component 
			float y = h/2 * sinf(theta);//calculate the y component 
			glVertex2f(x + cx, y + cy);//output vertex 
		}
		glEnd();
	}
}

//================||==================||==================||==================||==================>>

void makeMove(enumSquare origin, enumSquare target, Colour c, Type t) {
	if (c == white) {
		Bp.set(target, 0); Bn.set(target, 0); Bb.set(target, 0);
		Br.set(target, 0); Bq.set(target, 0); Bk.set(target, 0);
	}
	else {
		Wp.set(target, 0); Wn.set(target, 0); Wb.set(target, 0);
		Wr.set(target, 0); Wq.set(target, 0); Wk.set(target, 0);
	}

	switch (t) {
	case p:
		if (c == white) {
			Wp.set(origin, 0);
			Wp.set(target, 1);
		}
		else {
			Bp.set(origin, 0);
			Bp.set(target, 1);
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
			Wk.set(origin, 0);
			Wk.set(target, 1);
		}
		else {
			Bk.set(origin, 0);
			Bk.set(target, 1);
		}
		break;
	}
	Wpieces = Wp | Wn | Wb | Wr | Wq | Wk;
	Bpieces = Bp | Bn | Bb | Br | Bq | Bk;
	Occupied = Wpieces | Bpieces;
}

//returns the type of the piece in the entered square
Type getPieceType(int square) {
	if (Wp.test(square) || Bp.test(square)) return p;
	if (Wr.test(square) || Br.test(square)) return r;
	if (Wn.test(square) || Bn.test(square)) return n;
	if (Wb.test(square) || Bb.test(square)) return b;
	if (Wq.test(square) || Bq.test(square)) return q;
	if (Wk.test(square) || Bk.test(square)) return k;
	return None;
}

//returns the colour of the piece in the entered square
Colour getPieceColour(int square) {
	if (Wpieces.test(square)) return white;
	if (Bpieces.test(square)) return black;
	return NA;
}

//================||==================||==================||==================||==================>>

bitset<64> getLegalMoves(enumSquare sq, Colour c, Type t) {
	bitset<64> moves(0);
	switch (t) {
	case p:
		if (c == white) {
			moves.set(sq + 8, 1);
			if(sq < 16 && !Occupied.test(sq+8))
				moves.set(sq + 16, 1);
		}
		else {
			moves.set(sq - 8, 1);
			if (sq >= 48 && !Occupied.test(sq - 8))
				moves.set(sq - 16, 1);
		}
		break;
	case n:
		moves = nA.getKnightAttacks(sq) & ~Occupied;
		break;
	case b:
		moves = sA.getBishopAttacks(Occupied, sq) & ~Occupied;
		break;
	case r:
		moves = sA.getRookAttacks(Occupied, sq) & ~Occupied;
		break;
	case q:
		moves = sA.getQueenAttacks(Occupied, sq) & ~Occupied;
		break;
	case k:
		moves = kA.getKingAttacks(sq) & ~Occupied;
		break;
	}
	return moves;
}

bitset<64> getLegalCaptures(enumSquare sq, Colour c, Type t) {
	bitset<64> captures(0);
	bitset<64> opponentBB = c == white ? Bpieces : Wpieces;
	switch (t) {
	case p:
		if (c == white) {
			if(sq % 8 > 0 && opponentBB.test(sq + 7))
				captures.set(sq + 7, 1);
			if (sq % 8 < 7 && opponentBB.test(sq + 9))
				captures.set(sq + 9, 1);
		}
		else {
			if (sq % 8 > 0 && opponentBB.test(sq - 9))
				captures.set(sq - 9, 1);
			if (sq % 8 < 7 && opponentBB.test(sq - 7))
				captures.set(sq - 7, 1);
		}
		break;
	case n:
		captures = nA.getKnightAttacks(sq) & opponentBB;
		break;
	case b:
		captures = sA.getBishopAttacks(Occupied, sq) & opponentBB;
		break;
	case r:
		captures = sA.getRookAttacks(Occupied, sq) & opponentBB;
		break;
	case q:
		captures = sA.getQueenAttacks(Occupied, sq) & opponentBB;
		break;
	case k:
		captures = kA.getKingAttacks(sq) & opponentBB;
		break;
	}
	return captures;
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
	Wp.reset();	Wr.reset();
	Wn.reset();	Wb.reset();
	Wq.reset();	Wk.reset();

	Bp.reset();	Br.reset();
	Bn.reset();	Bb.reset();
	Bq.reset();	Bk.reset();

	nbPiecesOnBoard = 0;

	int pos = 0;
	for (char c : fen_info[0]) {
		if (isdigit(c)) {
			pos += int(c) - 48;
			continue;
		}
		nbPiecesOnBoard++;
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

	loadFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
	//loadFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");
	//loadFromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ");	
	//loadFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1 ");
	//loadFromFen("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
	//loadFromFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ");
	//loadFromFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");

	pA.setPawnAttacks();
	nA.setKnightAttacks();
	sA.setRayAttacks();
	kA.setKingAttacks();
	
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}