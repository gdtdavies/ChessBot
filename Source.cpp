#include<iostream>
#include<string>

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


int screenWidth = 900, screenHeight = 900;
int sqW, sqH;
glm::mat4 ViewMatrix;					// matrix for the modelling and viewing
glm::mat4 ProjectionMatrix;			// matrix for the orthographic projection

int nbPiecesOnBoard = 0;
Colour turn = white;

int HMcounter = 0, FMcounter = 0;

//-------------------------------------------------------------------------------------------------
void drawPiece(float x, float y, Colour c, Type t);
void drawBoard();


//=================================<<--------------------------->>================================//
//=================================<< START OF OPENGL FUNCTIONS >>================================//
//=================================<<--------------------------->>================================//

void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0); //sets the clear colour to black
	glLineWidth(2.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void display() {
	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT);

	ViewMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));

	glEnable(GL_BLEND);

	glm::mat4 ModelViewMatrix;

	drawBoard();

	drawPiece(0, 0, white, q);

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
	glutPostRedisplay();
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

void drawPiece(float x, float y, Colour c, Type t) {
	double w = (sqW * 0.75) / double(screenWidth);
	double h = (sqH * 0.75) / double(screenHeight);
	switch (t) {
	case p:
		drawLine(x, y - h / 2., x, y + h / 2., c);
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
		drawLine(x - w / 2., y + h / 2., x + w / 2., y - h / 2., c);
		drawLine(x - w / 2., y - h / 2., x + w / 2., y + h / 2., c);
		break;
	case r:
		drawLine(x - w / 2., y + h / 2., x + w / 2., y - h / 2., c);
		drawLine(x - w / 2., y - h / 2., x + w / 2., y + h / 2., c);
		break;
	case q:
		drawLine(x - w / 2., y + h / 2., x + w / 2., y - h / 2., c);
		drawLine(x - w / 2., y - h / 2., x + w / 2., y + h / 2., c);

		drawLine(x - w / 2., y, x + w / 2., y, c);
		drawLine(x, y - h / 2., x, y + h / 2., c);
		break;
	case k:
		drawLine(x - w / 2, y - h / 2, x + w / 2, y - h / 2, c);
		drawLine(x + w / 2, y - h / 2, x + w / 2, y + h / 2, c);
		drawLine(x + w / 2, y + h / 2, x - w / 2, y + h / 2, c);
		drawLine(x - w / 2, y + h / 2, x - w / 2, y - h / 2, c);
	}
}

void drawBoard() {
	double w = sqW / double(screenWidth);
	double h = sqH / double(screenHeight);
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++) {
			double x = i * w - 0.762;
			double y = j * h - 0.762;
			glBegin(GL_POLYGON);
			(i + j) % 2 == 0 ?
				glColor3f(1.0f, 0.75f, 0.5f):
				glColor3f(0.5f, 0.25f, 0.0f);
			glVertex2d(x - w/2., y - h/2.);
			glVertex2d(x - w/2., y + h/2.);
			glVertex2d(x + w/2., y + h/2.);
			glVertex2d(x + w/2., y - h/2.);
			glEnd();
		}
	}
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
	Wk.reset();	Wb.reset();
	Wq.reset();	Wk.reset();

	Bp.reset();	Br.reset();
	Bk.reset();	Bb.reset();
	Bq.reset();	Bk.reset();

	nbPiecesOnBoard = 0;

	int pos = 0;
	for (char c : fen_info[0]) {
		if (isdigit(c)) {
			pos += int(c) - 48;
			continue;
		}
		nbPiecesOnBoard++;
		c == 'P' ?
			Wp.set(pos, 1) :
			c == 'p' ?
			Bp.set(pos, 1) :
			c == 'R' ?
			Wr.set(pos, 1) :
			c == 'r' ?
			Br.set(pos, 1) :
			c == 'N' ?
			Wk.set(pos, 1) :
			c == 'n' ?
			Bk.set(pos, 1) :
			c == 'B' ?
			Wb.set(pos, 1) :
			c == 'b' ?
			Bb.set(pos, 1) :
			c == 'Q' ?
			Wq.set(pos, 1) :
			c == 'q' ?
			Bq.set(pos, 1) :
			c == 'K' ?
			Wk.set(pos, 1) :
			c == 'k' ?
			Bk.set(pos, 1) : __noop;
		pos++;
	}

	Wpieces = Wp| Wr| Wk| Wb| Wq| Wk;
	Bpieces = Bp| Br| Bk| Bb| Bq| Bk;

	Occupied = Wpieces | Bpieces;

	//=Active Colour=========================================================1=|
	if (fen_info[1] == "b")
		turn = black;
	else
		turn = white;

	//=Castling Rights=======================================================2=|
	castlingRights.reset();
	for (char c : fen_info[2]) {
		switch (c) {
		case 'K':
			castlingRights.set(0, 1);
			break;
		case 'Q':
			castlingRights.set(1, 1);
			break;
		case 'k':
			castlingRights.set(2, 1);
			break;
		case 'q':
			castlingRights.set(3, 1);
			break;
		}
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

	
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}