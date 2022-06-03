//ref: https://www.chessprogramming.org/Classical_Approach

class slidingAttacks {
private:
   bitset<64> rayAttacks[8][64];
   const enum enumDir { south_west, south, south_east, west, east, north_west, north, north_east };
	
   bool isNegative(int dir);
   int bitScan(bitset<64> mask, bool isNegative);
   bitset<64> getRayAttacks(bitset<64> occ, enumDir dir, enumSquare sq);

   bitset<64> diagonalAttacks(bitset<64> occ, enumSquare sq);
   bitset<64> antiDiagAttacks(bitset<64> occ, enumSquare sq);
   bitset<64> fileAttacks(bitset<64> occ, enumSquare sq);
   bitset<64> rankAttacks(bitset<64> occ, enumSquare sq);

public:
   void setRayAttacks();
	
   bitset<64> getRookAttacks(bitset<64> occ, enumSquare sq);
   bitset<64> getBishopAttacks(bitset<64> occ, enumSquare sq);
   bitset<64> getQueenAttacks(bitset<64> occ, enumSquare sq);
};

//populates ray
void slidingAttacks::setRayAttacks() {
   for (int origin = 0; origin < 64; origin++) {
      for (int target = 0; target < 64; target++)
      {
         if (target == origin) continue; //you can't attack the square you are on

         int origin_file = origin % 8;
         int target_file = target % 8;

         //east
         if (origin < target && target < origin + (8 - origin_file))//pos is between square and the right edge of the board
            rayAttacks[east][origin].set(target, 1);

         //west
         if (origin - origin_file <= target && target < origin) //pos is between square and the left edge of the board
            rayAttacks[west][origin].set(target, 1);

         //north-west / south-east
         if (abs(target - origin) % 7 == 0) {
            if (target < origin && origin_file < target_file)
               rayAttacks[south_east][origin].set(target, 1);
            else if (origin < target && target_file < origin_file)
               rayAttacks[north_west][origin].set(target, 1);
         }

         //north / south
         if (abs(target - origin) % 8 == 0) {
            if (target < origin)
               rayAttacks[south][origin].set(target, 1);
            else if (origin < target)
               rayAttacks[north][origin].set(target, 1);
         }

         //north-east / south-west
         if (abs(target - origin) % 9 == 0) {
            if (target < origin && target_file < origin_file)
               rayAttacks[south_west][origin].set(target, 1);
            else if (origin < target && origin_file < target_file)
               rayAttacks[north_east][origin].set(target, 1);
         }
      }
   }
}

//----------------------------------------------------RAY ATTACKS----------------------------------------------------//

bool slidingAttacks::isNegative(int dir) {
   assert(dir >= 0);
   assert(dir < 8);
   if (dir <= 3)
      return true;
   else
      return false;
}

int slidingAttacks::bitScan(bitset<64> mask, bool isNegative) {
   unsigned long index;
   if (isNegative)
      _BitScanReverse64(&index, mask.to_ullong());
   else
      _BitScanForward64(&index, mask.to_ullong());
   return index;
}

bitset<64> slidingAttacks::getRayAttacks(bitset<64> occ, enumDir dir, enumSquare sq) {
   bitset<64> attacks = rayAttacks[dir][sq];
   bitset<64> blockers = attacks & occ;
   if (!blockers.none()) {
      int blockingSquare = bitScan(blockers, isNegative(dir));
      attacks ^= rayAttacks[dir][blockingSquare];
   }
   return attacks;
}

//----------------------------------------------------LINE ATTACKS---------------------------------------------------//

bitset<64> slidingAttacks::diagonalAttacks(bitset<64> occ, enumSquare sq) {
   return getRayAttacks(occ, north_east, sq) | getRayAttacks(occ, south_west, sq);
}

bitset<64> slidingAttacks::antiDiagAttacks(bitset<64> occ, enumSquare sq) {
   return getRayAttacks(occ, north_west, sq) | getRayAttacks(occ, south_east, sq);
}

bitset<64> slidingAttacks::fileAttacks(bitset<64> occ, enumSquare sq) {
   return getRayAttacks(occ, north, sq) | getRayAttacks(occ, south, sq);
}

bitset<64> slidingAttacks::rankAttacks(bitset<64> occ, enumSquare sq) {
   return getRayAttacks(occ, east, sq) | getRayAttacks(occ, west, sq);
}

//---------------------------------------------------PIECE ATTACKS---------------------------------------------------//

bitset<64> slidingAttacks::getRookAttacks(bitset<64> occ, enumSquare sq) {
   return fileAttacks(occ, sq) | rankAttacks(occ, sq);
}

bitset<64> slidingAttacks::getBishopAttacks(bitset<64> occ, enumSquare sq) {
   return diagonalAttacks(occ, sq) | antiDiagAttacks(occ, sq);
}

bitset<64> slidingAttacks::getQueenAttacks(bitset<64> occ, enumSquare sq) {
   return getRookAttacks(occ, sq) | getBishopAttacks(occ, sq);
}