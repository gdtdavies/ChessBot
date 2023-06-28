bitset<64> Wp(0), Bp(0);
bitset<64> Wn(0), Bn(0);
bitset<64> Wb(0), Bb(0);
bitset<64> Wr(0), Br(0);
bitset<64> Wq(0), Bq(0);
bitset<64> Wk(0), Bk(0);

bitset<64> Wpieces = Wp | Wn | Wb | Wr | Wq | Wk;
bitset<64> Bpieces = Bp | Bn | Bb | Br | Bq | Bk;

bitset<64> Occupied = Wpieces | Bpieces;

bitset<64> EPTargets(0);
bitset<64> pEPTargets(0);

bitset<4> castlingRights(15);
bitset<4> pCastlingRights(15);


constexpr bitset<64> DarkSquares(0xAA55AA55AA55AA55);
constexpr bitset<64> LightSquares(0x55AA55AA55AA55AA);

constexpr bitset<64> Afile(0x0101010101010101);
constexpr bitset<64> Bfile(0x0202020202020202);
constexpr bitset<64> Cfile(0x0404040404040404);
constexpr bitset<64> Dfile(0x0808080808080808);
constexpr bitset<64> Efile(0x1010101010101010);
constexpr bitset<64> Ffile(0x2020202020202020);
constexpr bitset<64> Gfile(0x4040404040404040);
constexpr bitset<64> Hfile(0x8080808080808080);

constexpr bitset<64> Files[8] = { Afile, Bfile, Cfile, Dfile, Efile, Ffile, Gfile, Hfile };

constexpr bitset<64> Rank1(0xFF);
constexpr bitset<64> Rank2(0xFF00);
constexpr bitset<64> Rank3(0xFF0000);
constexpr bitset<64> Rank4(0xFF000000);
constexpr bitset<64> Rank5(0xFF00000000);
constexpr bitset<64> Rank6(0xFF0000000000);
constexpr bitset<64> Rank7(0xFF000000000000);
constexpr bitset<64> Rank8(0xFF00000000000000);

constexpr bitset<64> Ranks[8] = { Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8 };

constexpr bitset<64> QueenSide(0xF0F0F0F0F0F0F0F);
constexpr bitset<64> CenterFiles(0x3C3C3C3C3C3C3C3C);
constexpr bitset<64> KingSide(0xF0F0F0F0F0F0F0F0);
constexpr bitset<64> Center(0x3C3C3C3C0000);