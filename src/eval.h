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


constexpr int EG_FACTOR_PIECE_VALS[5] = {33, 370, 373, 675, 1574};
constexpr int EG_FACTOR_ALPHA = 2210;
constexpr int EG_FACTOR_BETA = 6350;
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
    {100, 396, 438, 681, 1349},
    {134, 407, 451, 746, 1441}
};
constexpr int KNOWN_WIN = PIECE_VALUES[EG][PAWNS] * 75;
constexpr int TB_WIN = PIECE_VALUES[EG][PAWNS] * 125;

//------------------------------Piece tables--------------------------------
constexpr int pieceSquareTable[2][6][32] = {
// Midgame
{
{ // Pawns
  0,  0,  0,  0,
 18, 10, 28, 42,
  8, 15, 30, 35,
 -2,  5,  2, 16,
-12, -4,  2,  9,
-10, -1,  0,  2,
 -6,  6, -1,  0,
  0,  0,  0,  0
},
{ // Knights
-128,-44,-37,-32,
-26,-16, -1, 14,
 -5,  7, 17, 32,
 12, 10, 26, 30,
  5, 10, 18, 22,
-13,  6,  6, 16,
-17,-10, -6,  3,
-50,-16,-11, -8
},
{ // Bishops
-16,-20,-15,-15,
-20,-15,-10, -8,
 10,  5,  1,  2,
  0, 12,  5, 15,
  5,  6,  6, 16,
  1, 10, -3,  8,
  5,  3, 10,  2,
-10,  3, -5, -2
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
-25,-21,-10, -5,
-13,-24, -9, -8,
 -8,  0,  0,  2,
 -5, -3, -3, -6,
 -3,  0, -3, -6,
 -6,  5, -1, -2,
-10,  2,  4,  2,
-16,-16,-10, -2
},
{ // Kings
-37,-32,-34,-45,
-34,-28,-32,-38,
-32,-24,-28,-30,
-31,-27,-30,-31,
-35,-20,-32,-32,
 -9, 20,-17,-23,
 35, 52,  9,-14,
 34, 59, 21,-10
}
},
// Endgame
{
{ // Pawns
  0,  0,  0,  0,
 28, 28, 30, 30,
 26, 26, 20, 20,
  8,  8,  2,  2,
 -5, -3, -2, -2,
-12, -3,  0,  0,
-12, -3,  2,  2,
  0,  0,  0,  0
},
{ // Knights
-65,-27,-18, -7,
-10,  0,  6, 10,
  0,  5, 13, 18,
  4, 11, 18, 25,
  0,  9, 16, 24,
 -7,  3,  7, 17,
-10,  0, -3,  6,
-31,-14, -8,  0
},
{ // Bishops
-12,-10, -7, -4
 -8, -7,  0,  0,
 -2,  2,  0,  1,
 -3,  2,  3,  1,
 -3,  0,  2,  2,
 -5, -1,  0,  2,
 -8, -6, -3, -2,
-13,-12,  0, -2
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
-14, -5, -1, -1,
 -6,  5, 10, 16,
 -2, 13, 18, 22,
  0, 16, 20, 26,
  0, 16, 20, 24,
 -4,  4,  8, 10,
-19,-14,-12, -8,
-26,-23,-23,-18
},
{ // Kings
-68,-18,-14, -7,
-12, 20, 28, 28,
  7, 34, 40, 42,
 -8, 25, 34, 36,
-13, 14, 24, 27,
-20, -2, 10, 14,
-26, -7,  4,  6,
-64,-36,-20,-17
}
}
};

//-------------------------Material eval constants------------------------------
constexpr int BISHOP_PAIR_VALUE = 56;
constexpr int TEMPO_VALUE = 21;

// Material imbalance terms
constexpr int OWN_OPP_IMBALANCE[2][5][5] = {
{
//       Opponent's
//    P   N   B   R   Q
    { 0},                   // Own pawns
    { 2,  0},               // Own knights
    {-1, -3,  0},           // Own bishops
    {-5, -5,-16,  0},       // Own rooks
    {11,-10, -8,-17,  0}    // Own queens
},
{
    { 0},                   // Own pawns
    { 6,  0},               // Own knights
    { 5,  6,  0},           // Own bishops
    { 1,-15,-21,  0},       // Own rooks
    {13,  0,  7, 27,  0}    // Own queens
}
};

// Bonus for knight in closed positions
constexpr int KNIGHT_CLOSED_BONUS[2] = {1, 4};

//------------------------Positional eval constants-----------------------------
// SPACE_BONUS[0][0] = behind own pawn, not center files
// SPACE_BONUS[0][1] = behind own pawn, center files
// SPACE_BONUS[1][0] = in front of opp pawn, not center files
// SPACE_BONUS[1][1] = in front of opp pawn, center files
constexpr int SPACE_BONUS[2][2] = {{12, 37}, {0, 10}};

// Mobility tables
constexpr int mobilityTable[2][5][28] = {
// Midgame
{
{ // Knights
-60, -9, 12, 23, 30, 34, 37, 40, 46},
{ // Bishops
-46,-17,  0, 10, 18, 22, 25, 29, 31, 33, 39, 43, 49, 53},
{ // Rooks
-97,-55,-18, -6, -2,  3,  7, 11, 15, 19, 22, 25, 27, 29, 32},
{ // Queens
-98,-80,-60,-37,-26,-17,-11, -8, -5, -3, -1,  2,  5,  7,
 10, 12, 15, 17, 19, 21, 23, 25, 26, 27, 29, 30, 31, 32},
{ // Kings
-20, 14, 25, 16, 11,  6, -2, -6, -5}
},

// Endgame
{
{ // Knights
-98,-49, -4,  8, 18, 26, 30, 32, 33},
{ // Bishops
-98,-53,-20,  3, 12, 22, 26, 31, 35, 38, 42, 45, 47, 48},
{ // Rooks
-102,-63, -4, 25, 36, 48, 55, 61, 67, 72, 77, 81, 86, 90, 94},
{ // Queens
-105,-82,-66,-44,-29,-20,-11, -2,  4, 10, 15, 18, 20, 23,
 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51},
{ // Kings
-50,-14,  0, 17, 18, 13, 18, 17,  6}
}
};

// Value of each square in the extended center in cp
constexpr Score EXTENDED_CENTER_VAL = E(2, 0);
// Additional bonus for squares in the center four squares in cp, in addition
// to EXTENDED_CENTER_VAL
constexpr Score CENTER_BONUS = E(4, 0);

// King safety
// The value of having 0, 1, and both castling rights
constexpr int CASTLING_RIGHTS_VALUE[3] = {0, 30, 70};
// The value of a pawn shield per pawn. First rank value is used for the
// penalty when the pawn is missing.
constexpr int PAWN_SHIELD_VALUE[4][8] = {
    {-15, 22, 25, 11,  6,  7,  3,  0}, // open h file, h2, h3, ...
    {-20, 39, 24,  0, -6,  2,  2,  0}, // g/b file
    {-17, 38,  2, -6, -5, -3,  3,  0}, // f/c file
    { -6, 14,  8,  5, -5,-10, -5,  0}  // d/e file
};
// Array for pawn storm values. Rank 1 of open is used for penalty
// when there is no opposing pawn
constexpr int PAWN_STORM_VALUE[3][4][8] = {
// Open file
{
    {14,-24, 35, 21, 15,  0,  0,  0},
    {17,-23, 56, 16,  9,  0,  0,  0},
    {10, 15, 53, 27, 19,  0,  0,  0},
    {11,  0, 30, 19, 14,  0,  0,  0}
},
// Blocked pawn
{
    { 0,  0, 26,  1,  0,  0,  0,  0},
    { 0,  0, 62,  3,  1,  0,  0,  0},
    { 0,  0, 66,  4,  0,  0,  0,  0},
    { 0,  0, 57, 11,  3,  0,  0,  0}
},
// Non-blocked pawn
{
    { 0, -2, 26, 16,  3,  0,  0,  0},
    { 0, -8, 28, 17, 12,  0,  0,  0},
    { 0, -1, 37, 21, 11,  0,  0,  0},
    { 0, -3, 10, 22,  7,  0,  0,  0}
},
};
// Penalty when the enemy king can use a storming pawn as protection
constexpr int PAWN_STORM_SHIELDING_KING = -139;

// Scale factor for pieces attacking opposing king
constexpr int KS_ARRAY_FACTOR = 128;
constexpr int KING_THREAT_MULTIPLIER[4] = {8, 5, 7, 3};
constexpr int KING_THREAT_SQUARE[4] = {8, 10, 7, 10};
constexpr int KING_DEFENSELESS_SQUARE = 24;
constexpr int KS_PAWN_FACTOR = 10;
constexpr int KING_PRESSURE = 3;
constexpr int KS_KING_PRESSURE_FACTOR = 24;
constexpr int KS_NO_KNIGHT_DEFENDER = 15;
constexpr int KS_NO_BISHOP_DEFENDER = 15;
constexpr int KS_BISHOP_PRESSURE = 8;
constexpr int KS_NO_QUEEN = -44;
constexpr int KS_BASE = -18;
constexpr int SAFE_CHECK_BONUS[4] = {56, 25, 65, 53};

// Minor pieces
// A penalty for each own pawn that is on a square of the same color as your bishop
constexpr Score BISHOP_PAWN_COLOR_PENALTY = E(-8, -6);
constexpr Score BISHOP_RAMMED_PAWN_COLOR_PENALTY = E(-3, -9);
// Minors shielded by own pawn in front
constexpr Score SHIELDED_MINOR_BONUS = E(13, 0);
// A bonus for strong outpost knights
constexpr Score KNIGHT_OUTPOST_BONUS = E(29, 23);
constexpr Score KNIGHT_OUTPOST_PAWN_DEF_BONUS = E(23, 9);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_BONUS = E(9, 14);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(14, 12);
// A smaller bonus for bishops
constexpr Score BISHOP_OUTPOST_BONUS = E(27, 18);
constexpr Score BISHOP_OUTPOST_PAWN_DEF_BONUS = E(26, 14);
constexpr Score BISHOP_POTENTIAL_OUTPOST_BONUS = E(6, 12);
constexpr Score BISHOP_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(17, 7);
// A bonus for fianchettoed bishops that are not blocked by pawns
constexpr Score BISHOP_FIANCHETTO_BONUS = E(26, 0);

// Rooks
constexpr Score ROOK_OPEN_FILE_BONUS = E(37, 11);
constexpr Score ROOK_SEMIOPEN_FILE_BONUS = E(22, 1);
constexpr Score ROOK_PAWN_RANK_THREAT = E(7, 14);

// Threats
constexpr Score UNDEFENDED_PAWN = E(-1, -17);
constexpr Score UNDEFENDED_MINOR = E(-21, -40);
constexpr Score PAWN_PIECE_THREAT = E(-75, -31);
constexpr Score MINOR_ROOK_THREAT = E(-71, -20);
constexpr Score MINOR_QUEEN_THREAT = E(-71, -33);
constexpr Score ROOK_QUEEN_THREAT = E(-78, -34);

constexpr Score LOOSE_PAWN = E(-14, -2);
constexpr Score LOOSE_MINOR = E(-15, -6);

// Pawn structure
// Passed pawns
constexpr Score PASSER_BONUS[8] = {E(  0,   0), E(  1,   5), E(  1, 5), E( 10,  18),
                                   E( 30,  27), E( 60,  54), E(114,118), E(  0,   0)};
constexpr Score PASSER_FILE_BONUS[8] = {E( 15, 17), E(  8, 11), E( -8,  1), E(-12, -7),
                                        E(-12, -7), E( -8,  1), E(  8, 11), E( 15, 17)};
constexpr Score FREE_PROMOTION_BONUS = E(8, 24);
constexpr Score FREE_STOP_BONUS = E(6, 11);
constexpr Score FULLY_DEFENDED_PASSER_BONUS = E(10, 14);
constexpr Score DEFENDED_PASSER_BONUS = E(9, 9);
constexpr Score OWN_KING_DIST = E(0, 3);
constexpr Score OPP_KING_DIST = E(0, 7);

// Doubled pawns
constexpr Score DOUBLED_PENALTY = E(-3, -20);
// Isolated pawns
constexpr Score ISOLATED_PENALTY = E(-15, -8);
constexpr Score ISOLATED_SEMIOPEN_PENALTY = E(-8, -13);
// Backward pawns
constexpr Score BACKWARD_PENALTY = E(-9, -7);
constexpr Score BACKWARD_SEMIOPEN_PENALTY = E(-20, -12);
// Undefended pawns that are not backwards or isolated
constexpr Score UNDEFENDED_PAWN_PENALTY = E(-6, -2);
// Pawn phalanxes
constexpr Score PAWN_PHALANX_BONUS[8] = {E( 0,  0), E( 5,  2), E( 5,  2), E(12,  9),
                                         E(29, 22), E(54, 44), E(75, 74), E( 0,  0)};
// Connected pawns
constexpr Score PAWN_CONNECTED_BONUS[8] = {E( 0,  0), E( 0,  0), E(14,  5), E( 7,  6),
                                           E(16, 12), E(37, 32), E(68, 62), E( 0,  0)};
// King-pawn tropism
constexpr int KING_TROPISM_VALUE = 17;

// Endgame win probability adjustment
constexpr int PAWN_ASYMMETRY_BONUS = 3;
constexpr int PAWN_COUNT_BONUS = 5;
constexpr int KING_OPPOSITION_DISTANCE_BONUS = 2;
constexpr int ENDGAME_BASE = -38;

// Scale factors for drawish endgames
constexpr int MAX_SCALE_FACTOR = 32;
constexpr int OPPOSITE_BISHOP_SCALING[2] = {13, 29};
constexpr int PAWNLESS_SCALING[4] = {1, 4, 8, 23};


#undef E

#endif
