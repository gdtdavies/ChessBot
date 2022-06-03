class kingAttacks {
private:
	bitset<64> attacks[64];

public:
	void setKingAttacks();
	bitset<64> getKingAttacks(int pos);
};


void kingAttacks::setKingAttacks() {
	for (int square = 0; square < 64; square++) {
		if (square % 8 > 0) {								//not on the a file
			attacks[square].set(square - 1, 1);	//west
			if (square >= 8)									//not on the first rank
				attacks[square].set(square - 9, 1);//soWe
		}
		if (square >= 8) {									//not on the 1st rank
			attacks[square].set(square - 8, 1);	//sout
			if (square % 8 < 7)								//not on the h file
				attacks[square].set(square - 7, 1);//soEa
		}
		if (square % 8 < 7) {								//not on the h file
			attacks[square].set(square + 1, 1);	//east
			if (square <= 63 - 8)							//not on the 8th rank
				attacks[square].set(square + 9, 1);//noEa
		}
		if (square <= 63 - 8) {								//not on the 8th rank
			attacks[square].set(square + 8, 1);	//nort
			if (square % 8 > 0)								//not on the a file
				attacks[square].set(square + 7, 1);//noWe
		}
	}
}

bitset<64> kingAttacks::getKingAttacks(int pos) {
	return attacks[pos];
}