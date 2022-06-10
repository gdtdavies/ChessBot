class Move {
private:
	enumSquare origin = null;
	enumSquare destination = null;
	Type type = None;
	Colour colour = NA;
	Type takenType = None;
	Promotion promotion = toNone;
public:
	Move();
	Move(enumSquare o, enumSquare d, Colour c, Type t);
	Move(enumSquare o, enumSquare d, Colour c, Type t, Type tt);
	Move(enumSquare o, enumSquare d, Colour c, Type t, Promotion promo);
	Move(enumSquare o, enumSquare d, Colour c, Type t, Type tt, Promotion promo);
	

	enumSquare getOrigin();
	enumSquare getDestination();
	Type getType();
	Colour getColour();
	Type getTakenType();
	Promotion getPromotion();
	string getMoveCode();

	void setTakenType(Type tt);
	void setPromotion(Promotion promo);

	bool isTake();	
	bool isCastle = false;
	bool isEP = false;
};

Move::Move() {}
Move::Move(enumSquare o, enumSquare d, Colour c, Type t) {
	origin = o;
	destination = d;
	type = t;
	colour = c;
}
Move::Move(enumSquare o, enumSquare d, Colour c, Type t, Type tt) {
	origin = o;
	destination = d;
	type = t;
	colour = c;
	takenType = tt;
}
Move::Move(enumSquare o, enumSquare d, Colour c, Type t, Promotion promo) {
	origin = o;
	destination = d;
	type = t;
	colour = c;
	promotion = promo;
}
Move::Move(enumSquare o, enumSquare d, Colour c, Type t, Type tt, Promotion promo) {
	origin = o;
	destination = d;
	type = t;
	colour = c;
	takenType = tt;
	promotion = promo;
}


enumSquare Move::getOrigin() {
	return origin;
}
enumSquare Move::getDestination() {
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
Promotion Move::getPromotion() {
	return promotion;
}
string Move::getMoveCode() {
	string code = "";

	// attacker code
	if (type == r)
		code += "R";
	else if (type == n)
		code += "N";
	else if (type == b)
		code += "B";
	else if (type == q)
		code += "Q";
	else if (type == k)
		code += "K";

	// take code
	if (takenType != None || isEP) {
		if (type == p)
			code += char(origin % 8) + 97;
		code += "x";
	}

	// target code
	code += char(destination % 8) + 97;
	code += char(destination / 8) + 49;

	// promotion code
	if (promotion != toNone) {
		code += "=";
		if (promotion == toQ)
			code += "Q";
		else if (promotion == toR)
			code += "R";
		else if (promotion == toB)
			code += "B";
		else if (promotion == toN)
			code += "N";
	}

	// king code
	//if (colour != NA) {
	//	if (mate)
	//		code += "#";
	//	else if (check)
	//		code += "+";
	//}

	return code;
}


void Move::setTakenType(Type tt) {
	takenType = tt;
}
void Move::setPromotion(Promotion promo) {
	promotion = promo;
}


bool Move::isTake() {
	return takenType != None;
}

