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


constexpr int EG_FACTOR_PIECE_VALS[5] = {40, 369, 377, 681, 1568};
constexpr int EG_FACTOR_ALPHA = 2130;
constexpr int EG_FACTOR_BETA = 6360;
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
    {100, 411, 448, 699, 1363},
    {138, 399, 454, 746, 1462}
};
constexpr int KNOWN_WIN = PIECE_VALUES[EG][PAWNS] * 75;
constexpr int TB_WIN = PIECE_VALUES[EG][PAWNS] * 125;

//------------------------------Piece tables--------------------------------
constexpr int pieceSquareTable[2][6][32] = {
// Midgame
{
{ // Pawns
  0,  0,  0,  0,
 20, 14, 35, 44,
 11, 19, 32, 35,
 -2,  2,  6, 14,
-13, -7,  2, 10,
-11, -4,  0,  3,
 -7,  3, -1,  0,
  0,  0,  0,  0
},
{ // Knights
-122,-44,-37,-32,
-26,-14, -1, 10,
 -6,  4, 12, 25,
 12,  9, 26, 30,
  2,  9, 20, 24,
-11,  7,  8, 16,
-17, -8, -2,  7,
-52,-16,-11, -8
},
{ // Bishops
-20,-24,-18,-18,
-24,-17,-10,-10,
  7,  0,  1,  2,
  0, 12,  5, 12,
  2,  6,  6, 14,
  3, 12,  2,  7,
  4, 11, 10,  5,
-13,  0, -5,  2
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
-25,-21,-15, -8,
-16,-24,-10, -8,
 -5, -3,  0,  2,
 -5, -3, -3, -3,
 -3,  0, -3, -3,
 -3,  5, -1, -2,
-12,  1,  3,  2,
-16,-16,-10,  2
},
{ // Kings
-37,-32,-38,-44,
-34,-28,-32,-38,
-32,-24,-28,-30,
-31,-25,-30,-31,
-37,-24,-32,-32,
 -8, 13,-23,-25,
 37, 52, 12,-16,
 34, 64, 14,-10
}
},
// Endgame
{
{ // Pawns
  0,  0,  0,  0,
 30, 30, 30, 30,
 26, 26, 20, 20,
  8,  6,  2,  2,
 -5,  0, -2, -2,
-12, -3,  0,  0,
-12, -3,  2,  2,
  0,  0,  0,  0
},
{ // Knights
-62,-28,-17, -9,
-13,  0,  4,  8,
  0,  5, 13, 20,
  4, 11, 18, 25,
  0,  9, 16, 24,
 -7,  3,  7, 17,
-10,  0,  0,  6,
-31,-14, -8,  0
},
{ // Bishops
-15,-10, -7, -8
 -7, -4, -4, -2,
 -2,  2,  3,  0,
  2,  2,  0,  4,
 -3,  2,  2,  2,
 -5, -1,  5,  2,
 -8, -4, -2, -2,
-16, -8, -4,  0
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
-18, -9, -1, -1,
 -9,  5, 10, 16,
 -2, 13, 18, 22,
  0, 16, 20, 26,
  0, 16, 20, 24,
 -4,  4,  8, 10,
-19,-14,-12, -8,
-26,-23,-23,-18
},
{ // Kings
-68,-18,-14, -7,
-10, 20, 28, 28,
  8, 32, 38, 40,
 -8, 19, 28, 30,
-16, 10, 20, 22,
-20, -2, 10, 14,
-26, -7,  4,  6,
-64,-36,-20,-17
}
}
};

//-------------------------Material eval constants------------------------------
constexpr int BISHOP_PAIR_VALUE = 62;
constexpr int TEMPO_VALUE = 18;

// Material imbalance terms
constexpr int OWN_OPP_IMBALANCE[2][5][5] = {
{
//       Opponent's
//    P   N   B   R   Q
    { 0},                   // Own pawns
    { 3,  0},               // Own knights
    { 2, -6,  0},           // Own bishops
    { 1, -8,-19,  0},       // Own rooks
    {-3,-20,-12,-29,  0}    // Own queens
},
{
    { 0},                   // Own pawns
    { 6,  0},               // Own knights
    { 3, -3,  0},           // Own bishops
    { 4,-12,-15,  0},       // Own rooks
    {26,  0,  8, 30,  0}    // Own queens
}
};

// Bonus for knight in closed positions
constexpr int KNIGHT_CLOSED_BONUS[2] = {1, 8};

//------------------------Positional eval constants-----------------------------
// SPACE_BONUS[0][0] = behind own pawn, not center files
// SPACE_BONUS[0][1] = behind own pawn, center files
// SPACE_BONUS[1][0] = in front of opp pawn, not center files
// SPACE_BONUS[1][1] = in front of opp pawn, center files
constexpr int SPACE_BONUS[2][2] = {{15, 37}, {3, 16}};

// Mobility tables
constexpr int mobilityTable[2][5][28] = {
// Midgame
{
{ // Knights
-60, -9, 13, 24, 32, 36, 41, 46, 51},
{ // Bishops
-54,-26, -3,  8, 19, 23, 26, 29, 31, 33, 39, 43, 49, 55},
{ // Rooks
-99,-53,-19, -5,  0,  5,  7, 12, 15, 18, 20, 22, 24, 26, 28},
{ // Queens
-108,-86,-63,-41,-28,-17,-10, -8, -5, -3, -1,  2,  5,  7,
 10, 12, 15, 17, 19, 21, 23, 25, 26, 27, 29, 30, 31, 32},
{ // Kings
-21, 18, 28, 18, 10,  3,  1, -9, -7}
},

// Endgame
{
{ // Knights
-99,-45, -8, 10, 19, 26, 30, 32, 34},
{ // Bishops
-95,-49,-18,  3, 14, 21, 26, 31, 35, 38, 42, 45, 47, 48},
{ // Rooks
-108,-68, -8, 22, 36, 48, 55, 61, 67, 72, 77, 81, 86, 90, 94},
{ // Queens
-108,-82,-66,-44,-26,-17,-11, -3,  5, 11, 15, 18, 20, 23,
 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51},
{ // Kings
-58,-19,  0, 20, 21, 13, 20, 19,  0}
}
};

// Value of each square in the extended center in cp
constexpr Score EXTENDED_CENTER_VAL = E(3, 0);
// Additional bonus for squares in the center four squares in cp, in addition
// to EXTENDED_CENTER_VAL
constexpr Score CENTER_BONUS = E(4, 0);

// King safety
// The value of having 0, 1, and both castling rights
constexpr int CASTLING_RIGHTS_VALUE[3] = {0, 27, 68};
// The value of a pawn shield per pawn. First rank value is used for the
// penalty when the pawn is missing.
constexpr int PAWN_SHIELD_VALUE[4][8] = {
    {-17, 22, 26, 10,  3, 10,  9,  0}, // open h file, h2, h3, ...
    {-20, 38, 23,-10, -8, -4, -4,  0}, // g/b file
    {-14, 38,  2, -8, -8, -3,  3,  0}, // f/c file
    { -4, 15, 11,  8, -5,-12,-10,  0}  // d/e file
};
// Array for pawn storm values. Rank 1 of open is used for penalty
// when there is no opposing pawn
constexpr int PAWN_STORM_VALUE[3][4][8] = {
// Open file
{
    {18,-30, 35, 17, 10,  0,  0,  0},
    {15,-30, 59, 20,  7,  0,  0,  0},
    { 7, 15, 55, 33, 16,  0,  0,  0},
    {11, -3, 31, 19, 15,  0,  0,  0}
},
// Blocked pawn
{
    { 0,  0, 26,  1,  0,  0,  0,  0},
    { 0,  0, 64,  3,  0,  0,  0,  0},
    { 0,  0, 70,  4,  0,  0,  0,  0},
    { 0,  0, 48, 10,  4,  0,  0,  0}
},
// Non-blocked pawn
{
    { 0,  6, 31, 14,  3,  0,  0,  0},
    { 0,-10, 26, 14,  8,  0,  0,  0},
    { 0,  3, 33, 22,  9,  0,  0,  0},
    { 0, -8,  0, 22,  6,  0,  0,  0}
},
};
// Penalty when the enemy king can use a storming pawn as protection
constexpr int PAWN_STORM_SHIELDING_KING = -141;

// Scale factor for pieces attacking opposing king
constexpr int KS_ARRAY_FACTOR = 128;
constexpr int KING_THREAT_MULTIPLIER[4] = {8, 4, 8, 3};
constexpr int KING_THREAT_SQUARE[4] = {9, 11, 5, 9};
constexpr int KING_DEFENSELESS_SQUARE = 23;
constexpr int KS_PAWN_FACTOR = 11;
constexpr int KING_PRESSURE = 3;
constexpr int KS_KING_PRESSURE_FACTOR = 25;
constexpr int KS_NO_KNIGHT_DEFENDER = 16;
constexpr int KS_NO_BISHOP_DEFENDER = 15;
constexpr int KS_BISHOP_PRESSURE = 8;
constexpr int KS_NO_QUEEN = -41;
constexpr int KS_BASE = -15;
constexpr int SAFE_CHECK_BONUS[4] = {55, 26, 68, 50};

// Minor pieces
// A penalty for each own pawn that is on a square of the same color as your bishop
constexpr Score BISHOP_PAWN_COLOR_PENALTY = E(-2, -3);
constexpr Score BISHOP_RAMMED_PAWN_COLOR_PENALTY = E(-8, -10);
// Minors shielded by own pawn in front
constexpr Score SHIELDED_MINOR_BONUS = E(15, 0);
// A bonus for strong outpost knights
constexpr Score KNIGHT_OUTPOST_BONUS = E(34, 24);
constexpr Score KNIGHT_OUTPOST_PAWN_DEF_BONUS = E(28, 8);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_BONUS = E(11, 15);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(13, 12);
// A smaller bonus for bishops
constexpr Score BISHOP_OUTPOST_BONUS = E(24, 18);
constexpr Score BISHOP_OUTPOST_PAWN_DEF_BONUS = E(31, 16);
constexpr Score BISHOP_POTENTIAL_OUTPOST_BONUS = E(9, 13);
constexpr Score BISHOP_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(12, 5);
// A bonus for fianchettoed bishops that are not blocked by pawns
constexpr Score BISHOP_FIANCHETTO_BONUS = E(26, 0);

// Rooks
constexpr Score ROOK_OPEN_FILE_BONUS = E(43, 12);
constexpr Score ROOK_SEMIOPEN_FILE_BONUS = E(22, 1);
constexpr Score ROOK_PAWN_RANK_THREAT = E(2, 11);

// Threats
constexpr Score UNDEFENDED_PAWN = E(-2, -14);
constexpr Score UNDEFENDED_MINOR = E(-27, -47);
constexpr Score PAWN_PIECE_THREAT = E(-85, -30);
constexpr Score MINOR_ROOK_THREAT = E(-87, -28);
constexpr Score MINOR_QUEEN_THREAT = E(-87, -38);
constexpr Score ROOK_QUEEN_THREAT = E(-92, -36);

constexpr Score LOOSE_PAWN = E(-11, -2);
constexpr Score LOOSE_MINOR = E(-13, -8);

// Pawn structure
// Passed pawns
constexpr Score PASSER_BONUS[8] = {E(  0,   0), E(  0,   6), E(  0, 8), E( 10,  17),
                                   E( 29,  26), E( 58,  53), E(113,124), E(  0,   0)};
constexpr Score PASSER_FILE_BONUS[8] = {E( 17, 16), E(  9, 12), E( -6, -1), E(-12, -7),
                                        E(-12, -7), E( -6, -1), E(  9, 12), E( 17, 16)};
constexpr Score FREE_PROMOTION_BONUS = E(8, 25);
constexpr Score FREE_STOP_BONUS = E(6, 11);
constexpr Score FULLY_DEFENDED_PASSER_BONUS = E(10, 15);
constexpr Score DEFENDED_PASSER_BONUS = E(9, 9);
constexpr Score OWN_KING_DIST = E(0, 3);
constexpr Score OPP_KING_DIST = E(0, 7);

// Doubled pawns
constexpr Score DOUBLED_PENALTY = E(-3, -21);
// Isolated pawns
constexpr Score ISOLATED_PENALTY = E(-18, -10);
constexpr Score ISOLATED_SEMIOPEN_PENALTY = E(-2, -11);
// Backward pawns
constexpr Score BACKWARD_PENALTY = E(-8, -7);
constexpr Score BACKWARD_SEMIOPEN_PENALTY = E(-16, -11);
// Undefended pawns that are not backwards or isolated
constexpr Score UNDEFENDED_PAWN_PENALTY = E(-5, -3);
// Pawn phalanxes
constexpr Score PAWN_PHALANX_BONUS[8] = {E( 0,  0), E( 7,  0), E( 4,  2), E(12,  8),
                                         E(29, 19), E(58, 47), E(68, 77), E( 0,  0)};
// Connected pawns
constexpr Score PAWN_CONNECTED_BONUS[8] = {E( 0,  0), E( 0,  0), E(14,  5), E(12,  4),
                                           E(16, 10), E(36, 29), E(64, 58), E( 0,  0)};
// King-pawn tropism
constexpr int KING_TROPISM_VALUE = 18;

// Endgame win probability adjustment
constexpr int PAWN_ASYMMETRY_BONUS = 3;
constexpr int PAWN_COUNT_BONUS = 5;
constexpr int KING_OPPOSITION_DISTANCE_BONUS = 2;
constexpr int ENDGAME_BASE = -38;

// Scale factors for drawish endgames
constexpr int MAX_SCALE_FACTOR = 32;
constexpr int OPPOSITE_BISHOP_SCALING[2] = {14, 28};
constexpr int PAWNLESS_SCALING[4] = {2, 5, 9, 24};


#undef E

#endif
