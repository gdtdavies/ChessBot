class Board {
private:
	float eval;

public:

};

class TranspositionTable {
private:
	vector<Board> table;
public:
	TranspositionTable();
	void saveBoard();
	bool findBoard();
};

TranspositionTable::TranspositionTable(){}