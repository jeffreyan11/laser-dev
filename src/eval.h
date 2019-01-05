/*
    Laser, a UCI chess engine written in C++11.
    Copyright 2015-2018 Jeffrey An and Michael An

    Laser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Laser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Laser.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __EVAL_H__
#define __EVAL_H__

#include <cstring>
#include "board.h"
#include "common.h"

class Board;

void initEvalTables();
void initDistances();
void setMaterialScale(int s);
void setKingSafetyScale(int s);

struct EvalInfo {
    uint64_t attackMaps[2][5];
    uint64_t fullAttackMaps[2];
    uint64_t doubleAttackMaps[2];
    uint64_t rammedPawns[2];
    uint64_t openFiles;

    void clear() {
        std::memset(this, 0, sizeof(EvalInfo));
    }
};

class Eval {
public:
    template <bool debug = false> int evaluate(Board &b);

private:
    EvalInfo ei;
    uint64_t pieces[2][6];
    uint64_t allPieces[2];
    int pieceCounts[2][6];
    int playerToMove;

    // Eval helpers
    template <int attackingColor>
    int getKingSafety(Board &b, PieceMoveList &attackers, uint64_t kingSqs, int pawnScore, int kingFile);
    int checkEndgameCases();
    int scoreSimpleKnownWin(int winningColor);
    int scoreCornerDistance(int winningColor, int wKingSq, int bKingSq);
};


constexpr int EG_FACTOR_PIECE_VALS[5] = {39, 369, 374, 680, 1560};
constexpr int EG_FACTOR_ALPHA = 2180;
constexpr int EG_FACTOR_BETA = 6370;
constexpr int EG_FACTOR_RES = 1000;

// Eval scores are packed into an unsigned 32-bit integer during calculations
// (the SWAR technique)
typedef uint32_t Score;

// Encodes 16-bit midgame and endgame evaluation scores into a single int
#define E(mg, eg) ((Score) ((int32_t) (((uint32_t) eg) << 16) + ((int32_t) mg)))

// Retrieves the final evaluation score to return from the packed eval value
inline int decEvalMg(Score encodedValue) {
    return (int) (encodedValue & 0xFFFF) - 0x8000;
}

inline int decEvalEg(Score encodedValue) {
    return (int) (encodedValue >> 16) - 0x8000;
}

// Since we can only work with unsigned numbers due to carryover / twos-complement
// negative number issues, we make 2^15 the 0 point for each of the two 16-bit
// halves of Score
constexpr Score EVAL_ZERO = 0x80008000;

// Array indexing constants
constexpr int MG = 0;
constexpr int EG = 1;

// Material constants
constexpr int PIECE_VALUES[2][5] = {
    {100, 417, 455, 704, 1363},
    {137, 399, 456, 746, 1462}
};
constexpr int KNOWN_WIN = PIECE_VALUES[EG][PAWNS] * 75;
constexpr int TB_WIN = PIECE_VALUES[EG][PAWNS] * 125;

//------------------------------Piece tables--------------------------------
constexpr int pieceSquareTable[2][6][32] = {
// Midgame
{
{ // Pawns
  0,  0,  0,  0,
 18,  9, 31, 42,
 11, 17, 30, 31,
 -4,  2,  5, 12,
-13, -5,  2, 10,
-11, -3,  0,  2,
 -5,  5,  1,  2,
  0,  0,  0,  0
},
{ // Knights
-122,-44,-37,-32,
-28,-17, -5, 10,
 -5,  3, 12, 29,
 16,  7, 26, 32,
  2, 10, 20, 24,
-10,  8,  7, 16,
-17, -8, -2,  7,
-54,-16,-11, -8
},
{ // Bishops
-16,-20,-15,-15,
-20,-12,-10, -8,
  6,  0, -1,  2,
  0, 12,  5, 12,
  0,  6,  6, 14,
  1, 12,  5,  8,
  4, 10, 12,  5,
 -9,  2, -5,  3
},
{ // Rooks
 -5,  0,  0,  0,
  5, 10, 10, 10,
 -5,  0,  0,  0,
 -5,  0,  0,  0,
 -5,  0,  0,  0,
 -5,  0,  0,  0,
 -5,  0,  0,  0,
 -5,  0,  0,  0
},
{ // Queens
-25,-21,-15, -5,
-20,-24,-10, -9,
 -8, -3,  0,  2,
 -5, -3, -3, -6,
 -3,  0, -3, -6,
 -6,  6, -1, -2,
-14,  2,  5,  5,
-16,-18,-10,  4
},
{ // Kings
-37,-32,-34,-45,
-34,-28,-32,-38,
-32,-24,-28,-30,
-31,-27,-30,-31,
-35,-20,-32,-32,
 -7, 15,-18,-23,
 38, 52,  9,-16,
 32, 64, 14,-10
}
},
// Endgame
{
{ // Pawns
  0,  0,  0,  0,
 32, 32, 32, 30,
 26, 26, 18, 18,
  9,  6,  2,  2,
 -5, -3, -2, -2,
-12, -3,  0,  0,
-12, -3,  2,  2,
  0,  0,  0,  0
},
{ // Knights
-70,-34,-22,-11,
-14,  0,  4, 10,
 -3,  5, 13, 18,
  6, 11, 17, 25,
  0,  9, 16, 22,
 -7,  4,  7, 17,
-10,  1,  0,  6,
-32,-14, -8,  0
},
{ // Bishops
-15,-13, -7, -8
-11, -5, -4, -6,
 -2,  2,  0,  0,
  2,  2,  0,  2,
 -3,  0,  2,  2,
 -5, -1,  5,  2,
 -8, -4, -2, -2,
-15, -6, -2,  1
},
{ // Rooks
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0
},
{ // Queens
-16, -8, -1,  0,
 -6,  5, 10, 16,
  0, 13, 18, 22,
  4, 20, 24, 28,
  2, 18, 22, 26,
 -7,  0,  8, 10,
-20,-17,-12,-10,
-30,-26,-21,-18
},
{ // Kings
-71,-21,-14, -7,
-11, 20, 28, 28,
  4, 35, 40, 40,
-12, 24, 32, 34,
-18, 12, 22, 25,
-23, -2, 13, 17,
-29, -7,  5,  9,
-65,-36,-20,-17
}
}
};

//-------------------------Material eval constants------------------------------
constexpr int BISHOP_PAIR_VALUE = 62;
constexpr int TEMPO_VALUE = 21;

// Material imbalance terms
constexpr int OWN_OPP_IMBALANCE[2][5][5] = {
{
//       Opponent's
//    P   N   B   R   Q
    { 0},                   // Own pawns
    { 3,  0},               // Own knights
    { 2, -7,  0},           // Own bishops
    { 1, -7,-19,  0},       // Own rooks
    {-3,-23,-15,-29,  0}    // Own queens
},
{
    { 0},                   // Own pawns
    { 5,  0},               // Own knights
    { 3, -4,  0},           // Own bishops
    { 4,-12,-14,  0},       // Own rooks
    {26, -2,  8, 30,  0}    // Own queens
}
};

// Bonus for knight in closed positions
constexpr int KNIGHT_CLOSED_BONUS[2] = {0, 8};

//------------------------Positional eval constants-----------------------------
// SPACE_BONUS[0][0] = behind own pawn, not center files
// SPACE_BONUS[0][1] = behind own pawn, center files
// SPACE_BONUS[1][0] = in front of opp pawn, not center files
// SPACE_BONUS[1][1] = in front of opp pawn, center files
constexpr int SPACE_BONUS[2][2] = {{16, 40}, {4, 16}};

// Mobility tables
constexpr int mobilityTable[2][5][28] = {
// Midgame
{
{ // Knights
-63,-10, 13, 25, 32, 36, 41, 46, 51},
{ // Bishops
-54,-24, -1,  9, 19, 23, 26, 29, 31, 33, 39, 43, 49, 54},
{ // Rooks
-94,-50,-19, -5,  0,  5,  7, 12, 15, 18, 20, 22, 24, 26, 28},
{ // Queens
-109,-86,-63,-39,-30,-18,-11, -8, -5, -3, -1,  2,  5,  7,
 10, 12, 15, 17, 19, 21, 23, 25, 26, 27, 29, 30, 31, 32},
{ // Kings
-20, 15, 26, 18, 10,  2,  0, -7, -9}
},

// Endgame
{
{ // Knights
-96,-41, -8,  6, 20, 26, 30, 32, 34},
{ // Bishops
-99,-48,-15,  2, 12, 20, 26, 31, 35, 38, 42, 45, 47, 48},
{ // Rooks
-111,-68, -5, 22, 36, 48, 55, 61, 67, 72, 77, 81, 86, 91, 96},
{ // Queens
-117,-85,-69,-46,-26,-17, -9, -2,  4, 11, 15, 18, 20, 23,
 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51},
{ // Kings
-60,-21,  1, 18, 21, 11, 18, 16, -2}
}
};

// Value of each square in the extended center in cp
constexpr Score EXTENDED_CENTER_VAL = E(3, 0);
// Additional bonus for squares in the center four squares in cp, in addition
// to EXTENDED_CENTER_VAL
constexpr Score CENTER_BONUS = E(5, 0);

// King safety
// The value of having 0, 1, and both castling rights
constexpr int CASTLING_RIGHTS_VALUE[3] = {0, 23, 68};
// The value of a pawn shield per pawn. First rank value is used for the
// penalty when the pawn is missing.
constexpr int PAWN_SHIELD_VALUE[4][8] = {
    {-16, 24, 29, 14,  4,  9,  5,  0}, // open h file, h2, h3, ...
    {-21, 39, 23, -8, -6, -1,  0,  0}, // g/b file
    {-14, 39,  2, -6, -5, -3,  4,  0}, // f/c file
    { -5, 15, 11,  6, -5,-13, -9,  0}  // d/e file
};
// Array for pawn storm values. Rank 1 of open is used for penalty
// when there is no opposing pawn
constexpr int PAWN_STORM_VALUE[3][4][8] = {
// Open file
{
    {18,-27, 35, 19, 12,  0,  0,  0},
    {14,-26, 54, 16,  7,  0,  0,  0},
    { 9, 18, 56, 34, 18,  0,  0,  0},
    {10, -3, 33, 20, 13,  0,  0,  0}
},
// Blocked pawn
{
    { 0,  0, 28,  1,  0,  0,  0,  0},
    { 0,  0, 68,  3,  0,  0,  0,  0},
    { 0,  0, 70,  3,  0,  0,  0,  0},
    { 0,  0, 51, 10,  1,  0,  0,  0}
},
// Non-blocked pawn
{
    { 0,  3, 26, 18,  3,  0,  0,  0},
    { 0,-13, 24, 15,  8,  0,  0,  0},
    { 0,  3, 34, 23, 10,  0,  0,  0},
    { 0, -3,  5, 25,  7,  0,  0,  0}
},
};
// Penalty when the enemy king can use a storming pawn as protection
constexpr int PAWN_STORM_SHIELDING_KING = -141;

// Scale factor for pieces attacking opposing king
constexpr int KS_ARRAY_FACTOR = 128;
constexpr int KING_THREAT_MULTIPLIER[4] = {8, 5, 8, 3};
constexpr int KING_THREAT_SQUARE[4] = {9, 10, 7, 9};
constexpr int KING_DEFENSELESS_SQUARE = 24;
constexpr int KS_PAWN_FACTOR = 11;
constexpr int KING_PRESSURE = 3;
constexpr int KS_KING_PRESSURE_FACTOR = 24;
constexpr int KS_NO_KNIGHT_DEFENDER = 17;
constexpr int KS_NO_BISHOP_DEFENDER = 15;
constexpr int KS_BISHOP_PRESSURE = 10;
constexpr int KS_NO_QUEEN = -44;
constexpr int KS_BASE = -17;
constexpr int SAFE_CHECK_BONUS[4] = {53, 24, 68, 50};

// Minor pieces
// A penalty for each own pawn that is on a square of the same color as your bishop
constexpr Score BISHOP_PAWN_COLOR_PENALTY = E(-2, -4);
constexpr Score BISHOP_RAMMED_PAWN_COLOR_PENALTY = E(-8, -8);
// Minors shielded by own pawn in front
constexpr Score SHIELDED_MINOR_BONUS = E(17, 0);
// A bonus for strong outpost knights
constexpr Score KNIGHT_OUTPOST_BONUS = E(35, 24);
constexpr Score KNIGHT_OUTPOST_PAWN_DEF_BONUS = E(28, 11);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_BONUS = E(10, 16);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(15, 10);
// A smaller bonus for bishops
constexpr Score BISHOP_OUTPOST_BONUS = E(27, 20);
constexpr Score BISHOP_OUTPOST_PAWN_DEF_BONUS = E(29, 14);
constexpr Score BISHOP_POTENTIAL_OUTPOST_BONUS = E(9, 12);
constexpr Score BISHOP_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(15, 5);
// A bonus for fianchettoed bishops that are not blocked by pawns
constexpr Score BISHOP_FIANCHETTO_BONUS = E(26, 0);

// Rooks
constexpr Score ROOK_OPEN_FILE_BONUS = E(43, 12);
constexpr Score ROOK_SEMIOPEN_FILE_BONUS = E(22, 1);
constexpr Score ROOK_PAWN_RANK_THREAT = E(3, 11);

// Threats
constexpr Score UNDEFENDED_PAWN = E(-3, -15);
constexpr Score UNDEFENDED_MINOR = E(-29, -46);
constexpr Score PAWN_PIECE_THREAT = E(-86, -33);
constexpr Score MINOR_ROOK_THREAT = E(-87, -27);
constexpr Score MINOR_QUEEN_THREAT = E(-85, -46);
constexpr Score ROOK_QUEEN_THREAT = E(-94, -43);

constexpr Score LOOSE_PAWN = E(-12, -2);
constexpr Score LOOSE_MINOR = E(-14, -8);

// Pawn structure
// Passed pawns
constexpr Score PASSER_BONUS[8] = {E(  0,   0), E(  2,   6), E(  3, 7), E( 13,  19),
                                   E( 30,  23), E( 60,  54), E(115,123), E(  0,   0)};
constexpr Score PASSER_FILE_BONUS[8] = {E( 15, 16), E(  8, 12), E( -8, -1), E(-12, -7),
                                        E(-12, -7), E( -8, -1), E(  8, 12), E( 15, 16)};
constexpr Score FREE_PROMOTION_BONUS = E(9, 25);
constexpr Score FREE_STOP_BONUS = E(6, 11);
constexpr Score FULLY_DEFENDED_PASSER_BONUS = E(10, 15);
constexpr Score DEFENDED_PASSER_BONUS = E(10, 10);
constexpr Score OWN_KING_DIST = E(0, 3);
constexpr Score OPP_KING_DIST = E(0, 7);

// Doubled pawns
constexpr Score DOUBLED_PENALTY = E(-2, -19);
// Isolated pawns
constexpr Score ISOLATED_PENALTY = E(-17, -11);
constexpr Score ISOLATED_SEMIOPEN_PENALTY = E(-2, -10);
// Backward pawns
constexpr Score BACKWARD_PENALTY = E(-7, -9);
constexpr Score BACKWARD_SEMIOPEN_PENALTY = E(-18, -10);
// Undefended pawns that are not backwards or isolated
constexpr Score UNDEFENDED_PAWN_PENALTY = E(-5, -3);
// Pawn phalanxes
constexpr Score PAWN_PHALANX_BONUS[8] = {E( 0,  0), E( 8,  0), E( 4,  2), E(12,  6),
                                         E(29, 19), E(61, 45), E(75, 83), E( 0,  0)};
// Connected pawns
constexpr Score PAWN_CONNECTED_BONUS[8] = {E( 0,  0), E( 0,  0), E(14,  5), E(11,  4),
                                           E(15,  9), E(37, 31), E(61, 58), E( 0,  0)};
// King-pawn tropism
constexpr int KING_TROPISM_VALUE = 18;

// Endgame win probability adjustment
constexpr int PAWN_ASYMMETRY_BONUS = 4;
constexpr int PAWN_COUNT_BONUS = 6;
constexpr int KING_OPPOSITION_DISTANCE_BONUS = 2;
constexpr int ENDGAME_BASE = -39;

// Scale factors for drawish endgames
constexpr int MAX_SCALE_FACTOR = 32;
constexpr int OPPOSITE_BISHOP_SCALING[2] = {14, 28};
constexpr int PAWNLESS_SCALING[4] = {2, 5, 8, 23};


#undef E

#endif
