class pawnAttacks {
private:
	bitset<64> Wattacks[64];
	bitset<64> Battacks[64];

public:
	void setPawnAttacks();
	bitset<64> getPawnAttacks(int pos, Colour c);
};



void pawnAttacks::setPawnAttacks() {
	for (int sq = 0; sq < 64; sq++) {
		if (sq >= 8) {
			if (sq % 8 > 0) 
				Battacks[sq].set(sq - 9, 1);
			if (sq % 8 < 7)
				Battacks[sq].set(sq - 7, 1);
		}
		if (sq < 56) {
			if (sq % 8 > 0)
				Wattacks[sq].set(sq + 7, 1);
			if (sq % 8 < 7)
				Wattacks[sq].set(sq + 9, 1);
		}
	}
}

bitset<64> pawnAttacks::getPawnAttacks(int pos, Colour c) {
	if(c == white)
		return Wattacks[pos];
	return Battacks[pos];
}