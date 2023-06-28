class GUI {
private:
	void drawLine(float x1, float y1, float x2, float y2, Colour c);
public:
	void drawPiece(float x, float y, Colour c, Type t);
	void drawBoard();
	void drawSelectedSq(int rank, int file);
	void drawAttacks(bitset<64>);
	void drawPromotion();
};

void GUI::drawLine(float x1, float y1, float x2, float y2, Colour c) {
	glBegin(GL_LINES);

	c == white ? glColor3f(1.0, 1.0, 1.0) :
		c == black ? glColor3f(0.0, 0.0, 0.0) : glColor3f(0.75, 0.0, 0.0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void GUI::drawPiece(float rank, float file, Colour c, Type t) {
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

void GUI::drawBoard() {
	double w = sqW / double(screenWidth);
	double h = sqH / double(screenHeight);
	for (int file = 0; file < 8; file++) {
		for (int rank = 0; rank < 8; rank++) {
			double x = rank * w - 0.762;
			double y = file * h - 0.762;
			glBegin(GL_POLYGON);
			(file + rank) % 2 == 1 ?
				//glColor3f(0.80f, 0.65f, 0.50f):  
				//glColor3f(0.50f, 0.30f, 0.20f);  
				glColor3f(0.40f, 0.75f, 1.00f) :
				glColor3f(0.10f, 0.50f, 0.70f);
			glVertex2d(x - w / 2., y - h / 2.);
			glVertex2d(x - w / 2., y + h / 2.);
			glVertex2d(x + w / 2., y + h / 2.);
			glVertex2d(x + w / 2., y - h / 2.);
			glEnd();
		}
	}
}

void GUI::drawSelectedSq(int rank, int file) {
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

void GUI::drawAttacks(bitset<64> attacks) {
	for (int i = 0; i < 64; i++) {
		if (!attacks.test(i)) continue;
		int rank = i / 8;
		int file = i % 8;

		float w = (sqW / 2) / double(screenWidth);
		float h = (sqH / 2) / double(screenHeight);

		double cx = file * sqW / double(screenWidth) - 0.762;
		double cy = rank * sqH / double(screenHeight) - 0.762;

		glBegin(GL_LINE_LOOP);

		getPieceType(i) == None ? glColor3f(0.0f, 0.75f, 0.0f) : glColor3f(0.75f, 0.0f, 0.0f);
		for (int j = 0; j < 20; j++) {
			float theta = 2.0f * 3.1415926f * float(j) / float(20);//get the current angle 
			float x = w / 2 * cosf(theta);//calculate the x component 
			float y = h / 2 * sinf(theta);//calculate the y component 
			glVertex2f(x + cx, y + cy);//output vertex 
		}
		glEnd();
	}
}

void GUI::drawPromotion() {
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
