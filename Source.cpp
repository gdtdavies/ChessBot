#include<iostream>


//--- OpenGL ---
#include "GL\glew.h"
#include "GL\wglew.h"
#pragma comment(lib, "glew32.lib")
//--------------

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "GL\freeglut.h"

const enum Colour { white, black, NA };
const enum Type {p, r, n, b, q, k, None};


int screenWidth = 900, screenHeight = 900;
int sqW, sqH;
glm::mat4 ViewMatrix;					// matrix for the modelling and viewing
glm::mat4 ProjectionMatrix;			// matrix for the orthographic projection

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
	
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}