class Move {
private:
	int origin = null;
	int destination = null;
	Type type = None;
	Colour colour = NA;
	Type takenType = None;
public:
	Move();
	Move(int o, int d, Colour c, Type t);
	Move(int o, int d, Colour c, Type t, Type tt);

	int getOrigin();
	int getDestination();
	Type getType();
	Colour getColour();
	Type getTakenType();

	void setTakenType(Type tt);

	bool isTake();	
	bool isCastle = false;
	bool isEP = false;
};

Move::Move() {}
Move::Move(int o, int d, Colour c, Type t) {
	origin = o;
	destination = d;
	type = t;
	colour = c;
}
Move::Move(int o, int d, Colour c, Type t, Type tt) {
	origin = o;
	destination = d;
	type = t;
	colour = c;
	takenType = tt;
}

int Move::getOrigin() {
	return origin;
}
int Move::getDestination() {
	return destination;
}
Type Move::getType() {
	return type;
}
Colour Move::getColour() {
	return colour;
}
Type Move::getTakenType() {
	return takenType;
}

void Move::setTakenType(Type tt) {
	takenType = tt;
}

bool Move::isTake() {
	return takenType != None;
}