class knightAttacks {
private:
	bitset<64> attacks[64];

public:
	void setKnightAttacks();
	bitset<64> getKnightAttacks(int pos);
};


void knightAttacks::setKnightAttacks() {
	for (int square = 0; square < 64; square++) {

		if (square % 8 > 0) { // 1We
			if (square >= 16)
				attacks[square].set(square - 17, 1); // SoSoWe
			if (square <= 63 - 15)
				attacks[square].set(square + 15, 1); // NoNoWe
		}
		if (square % 8 > 1) { // 2We
			if (square >= 8)
				attacks[square].set(square - 10, 1); // SoWeWe
			if (square <= 63 - 8)
				attacks[square].set(square + 6, 1);	 // NoWeWe
		}
		if (square % 8 < 6) { // 2Ea
			if (square >= 8)
				attacks[square].set(square - 6, 1);  // SoEaEa
			if(square <= 63 - 8)
				attacks[square].set(square + 10, 1); // NoEaEa
		}
		if (square % 8 < 7) { // 1Ea
			if (square >= 16)
				attacks[square].set(square - 15, 1); // SoSoEa
			if (square <= 63 - 16)
				attacks[square].set(square + 17, 1); // NoNoEa
		}
	}	
}

bitset<64> knightAttacks::getKnightAttacks(int pos) {
	return attacks[pos];
}