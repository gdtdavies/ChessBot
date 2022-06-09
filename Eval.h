#pragma once

class Evaluator {
private:
	
public:
	float evaluate();
};

float Evaluator::evaluate() {
	if (isStaleMate(turn)) return 0;
	if (isCheckMate(turn)) return turn == white ? INFINITY : -INFINITY;

	float material
		= 9.00 * (int)(Wq.count() - Bq.count())
		+ 5.00 * (int)(Wr.count() - Br.count())
		+ 3.00 * (int)(Wb.count() - Bb.count())
		+ 3.00 * (int)(Wn.count() - Bk.count())
		+ 1.00 * (int)(Wp.count() - Bp.count());
	return material;
}
