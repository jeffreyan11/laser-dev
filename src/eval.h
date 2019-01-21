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


constexpr int EG_FACTOR_PIECE_VALS[5] = {39, 366, 370, 676, 1572};
constexpr int EG_FACTOR_ALPHA = 2210;
constexpr int EG_FACTOR_BETA = 6340;
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
    {100, 405, 445, 683, 1343},
    {135, 401, 454, 741, 1444}
};
constexpr int KNOWN_WIN = PIECE_VALUES[EG][PAWNS] * 75;
constexpr int TB_WIN = PIECE_VALUES[EG][PAWNS] * 125;

//------------------------------Piece tables--------------------------------
constexpr int pieceSquareTable[2][6][32] = {
// Midgame
{
{ // Pawns
  0,  0,  0,  0,
 21,  6, 32, 44,
  7, 12, 31, 39,
 -2,  2,  2, 18,
-12, -4,  2, 12,
 -9, -1,  0,  2,
 -5,  6, -1,  0,
  0,  0,  0,  0
},
{ // Knights
-128,-40,-37,-29,
-26,-11,  0, 15,
 -8,  7, 17, 27,
 15, 10, 26, 29,
  4,  8, 20, 24,
-11,  5,  3, 15,
-17,-12, -6,  4,
-55,-16,-13,-10
},
{ // Bishops
-23,-22,-18,-16,
-22,-18,-10, -8,
  9,  5,  1,  2,
  0, 16,  8, 14,
  4, 10,  6, 14,
  2, 14,  5,  6,
  6,  5, 10,  5,
-11,  5, -5, -1
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
-16,-21, -7, -6,
 -8, -3,  0,  2,
 -5, -3, -3, -3,
 -3,  0, -3, -3,
 -6,  5, -1, -2,
-12,  1,  3,  2,
-16,-16,-10,  2
},
{ // Kings
-37,-32,-34,-45,
-34,-28,-32,-38,
-32,-24,-28,-30,
-31,-25,-30,-31,
-35,-15,-32,-32,
 -7, 21,-15,-23,
 38, 50,  6,-14,
 35, 61, 26, -9
}
},
// Endgame
{
{ // Pawns
  0,  0,  0,  0,
 28, 28, 30, 30,
 24, 26, 20, 20,
  8,  6,  2,  2,
 -5, -2, -2, -2,
-12, -3,  0,  0,
-12, -3,  2,  2,
  0,  0,  0,  0
},
{ // Knights
-65,-28,-19, -8,
-15,  3,  6, 13,
  5,  9, 17, 22,
  9, 14, 22, 27,
  3, 11, 16, 21,
 -7,  3,  7, 14,
-10,  0, -3,  6,
-27,-11, -7,  0
},
{ // Bishops
-12,-10, -7, -8
 -7, -4, -4, -2,
 -2,  2,  3,  0,
  2,  2,  0,  4,
 -3,  0,  2,  2,
 -5, -1,  5,  2,
 -8, -4, -2, -2,
-13,-12, -4,  0
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
-74,-20,-17,-12,
-13, 20, 28, 28,
  8, 32, 38, 40,
 -3, 19, 28, 30,
-13, 10, 20, 22,
-20, -2, 11, 16,
-32,-12,  2,  4,
-70,-37,-25,-20
}
}
};

//-------------------------Material eval constants------------------------------
constexpr int BISHOP_PAIR_VALUE = 57;
constexpr int TEMPO_VALUE = 20;

// Material imbalance terms
constexpr int OWN_OPP_IMBALANCE[2][5][5] = {
{
//       Opponent's
//    P   N   B   R   Q
    { 0},                   // Own pawns
    { 3,  0},               // Own knights
    { 2, -4,  0},           // Own bishops
    {-2, -3,-14,  0},       // Own rooks
    { 1,-10, -3,-25,  0}    // Own queens
},
{
    { 0},                   // Own pawns
    { 5,  0},               // Own knights
    { 3, -3,  0},           // Own bishops
    { 1,-14,-17,  0},       // Own rooks
    {21,  2,  8, 27,  0}    // Own queens
}
};

// Bonus for knight in closed positions
constexpr int KNIGHT_CLOSED_BONUS[2] = {1, 7};

//------------------------Positional eval constants-----------------------------
// SPACE_BONUS[0][0] = behind own pawn, not center files
// SPACE_BONUS[0][1] = behind own pawn, center files
// SPACE_BONUS[1][0] = in front of opp pawn, not center files
// SPACE_BONUS[1][1] = in front of opp pawn, center files
constexpr int SPACE_BONUS[2][2] = {{14, 36}, {1, 10}};

// Mobility tables
constexpr int mobilityTable[2][5][28] = {
// Midgame
{
{ // Knights
-56, -9, 11, 21, 29, 34, 38, 42, 45},
{ // Bishops
-50,-20,  0,  8, 17, 22, 26, 29, 31, 33, 39, 43, 49, 55},
{ // Rooks
-102,-55,-19, -5,  0,  5,  7, 12, 15, 18, 21, 24, 27, 30, 33},
{ // Queens
-107,-83,-58,-39,-26,-16,-11, -7, -5, -3, -1,  2,  5,  7,
 10, 12, 15, 17, 19, 21, 23, 25, 26, 27, 29, 30, 31, 32},
{ // Kings
-20, 17, 22, 14,  8,  3, -2, -4, -6}
},

// Endgame
{
{ // Knights
-93,-46, -8, 11, 20, 26, 30, 32, 34},
{ // Bishops
-105,-48,-18,  3, 13, 21, 26, 31, 35, 38, 41, 44, 46, 47},
{ // Rooks
-110,-67, -6, 22, 37, 47, 54, 60, 66, 72, 77, 81, 86, 90, 94},
{ // Queens
-109,-85,-66,-44,-30,-19,-11, -2,  4, 10, 15, 18, 20, 23,
 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51},
{ // Kings
-43, -9,  2, 16, 18, 12, 18, 13,  2}
}
};

// Value of each square in the extended center in cp
constexpr Score EXTENDED_CENTER_VAL = E(2, 0);
// Additional bonus for squares in the center four squares in cp, in addition
// to EXTENDED_CENTER_VAL
constexpr Score CENTER_BONUS = E(5, 0);

// King safety
// The value of having 0, 1, and both castling rights
constexpr int CASTLING_RIGHTS_VALUE[3] = {0, 30, 72};
// The value of a pawn shield per pawn. First rank value is used for the
// penalty when the pawn is missing.
constexpr int PAWN_SHIELD_VALUE[4][8] = {
    {-15, 24, 26, 12,  8,  7,  4,  0}, // open h file, h2, h3, ...
    {-21, 38, 24, -3, -6, -1,  2,  0}, // g/b file
    {-17, 39,  2, -6, -5, -3,  3,  0}, // f/c file
    { -2, 16, 11,  8, -3, -8,  0,  0}  // d/e file
};
// Array for pawn storm values. Rank 1 of open is used for penalty
// when there is no opposing pawn
constexpr int PAWN_STORM_VALUE[3][4][8] = {
// Open file
{
    {16,-30, 34, 19, 14,  0,  0,  0},
    {18,-29, 52, 13,  7,  0,  0,  0},
    { 9, 12, 54, 29, 20,  0,  0,  0},
    {12, -6, 28, 12, 15,  0,  0,  0}
},
// Blocked pawn
{
    { 0,  0, 27,  1,  0,  0,  0,  0},
    { 0,  0, 60,  3,  0,  0,  0,  0},
    { 0,  0, 66,  4,  0,  0,  0,  0},
    { 0,  0, 63, 11,  4,  0,  0,  0}
},
// Non-blocked pawn
{
    { 0,  3, 25, 16,  3,  0,  0,  0},
    { 0,-12, 25, 17,  8,  0,  0,  0},
    { 0, -6, 33, 24,  6,  0,  0,  0},
    { 0, -6,  8, 25,  7,  0,  0,  0}
},
};
// Penalty when the enemy king can use a storming pawn as protection
constexpr int PAWN_STORM_SHIELDING_KING = -141;

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
constexpr Score BISHOP_PAWN_COLOR_PENALTY = E(-4, -4);
constexpr Score BISHOP_RAMMED_PAWN_COLOR_PENALTY = E(-7, -10);
// Minors shielded by own pawn in front
constexpr Score SHIELDED_MINOR_BONUS = E(13, 0);
// A bonus for strong outpost knights
constexpr Score KNIGHT_OUTPOST_BONUS = E(28, 23);
constexpr Score KNIGHT_OUTPOST_PAWN_DEF_BONUS = E(19, 8);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_BONUS = E(8, 16);
constexpr Score KNIGHT_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(15, 12);
// A smaller bonus for bishops
constexpr Score BISHOP_OUTPOST_BONUS = E(26, 18);
constexpr Score BISHOP_OUTPOST_PAWN_DEF_BONUS = E(26, 14);
constexpr Score BISHOP_POTENTIAL_OUTPOST_BONUS = E(7, 13);
constexpr Score BISHOP_POTENTIAL_OUTPOST_PAWN_DEF_BONUS = E(17, 7);
// A bonus for fianchettoed bishops that are not blocked by pawns
constexpr Score BISHOP_FIANCHETTO_BONUS = E(22, 0);

// Rooks
constexpr Score ROOK_OPEN_FILE_BONUS = E(37, 11);
constexpr Score ROOK_SEMIOPEN_FILE_BONUS = E(23, 1);
constexpr Score ROOK_PAWN_RANK_THREAT = E(5, 14);

// Threats
constexpr Score UNDEFENDED_PAWN = E(-2, -16);
constexpr Score UNDEFENDED_MINOR = E(-21, -40);
constexpr Score PAWN_PIECE_THREAT = E(-82, -30);
constexpr Score MINOR_ROOK_THREAT = E(-78, -27);
constexpr Score MINOR_QUEEN_THREAT = E(-78, -37);
constexpr Score ROOK_QUEEN_THREAT = E(-82, -38);

constexpr Score LOOSE_PAWN = E(-14, -2);
constexpr Score LOOSE_MINOR = E(-15, -6);

// Pawn structure
// Passed pawns
constexpr Score PASSER_BONUS[8] = {E(  0,   0), E(  1,   5), E(  1,  7), E(  9,  17),
                                   E( 30,  23), E( 62,  54), E(115,116), E(  0,   0)};
constexpr Score PASSER_FILE_BONUS[8] = {E( 18, 17), E( 11, 14), E(-12,  1), E(-14, -8),
                                        E(-14, -8), E(-12,  1), E( 11, 14), E( 18, 17)};
constexpr Score FREE_PROMOTION_BONUS = E(8, 24);
constexpr Score FREE_STOP_BONUS = E(6, 11);
constexpr Score FULLY_DEFENDED_PASSER_BONUS = E(10, 14);
constexpr Score DEFENDED_PASSER_BONUS = E(9, 9);
constexpr Score OWN_KING_DIST = E(0, 3);
constexpr Score OPP_KING_DIST = E(0, 7);

// Doubled pawns
constexpr Score DOUBLED_PENALTY = E(-1, -20);
// Isolated pawns
constexpr Score ISOLATED_PENALTY = E(-19, -13);
constexpr Score ISOLATED_SEMIOPEN_PENALTY = E(-3, -11);
// Backward pawns
constexpr Score BACKWARD_PENALTY = E(-11, -10);
constexpr Score BACKWARD_SEMIOPEN_PENALTY = E(-19, -13);
// Undefended pawns that are not backwards or isolated
constexpr Score UNDEFENDED_PAWN_PENALTY = E(-7, -3);
// Pawn phalanxes
constexpr Score PAWN_PHALANX_BONUS[8] = {E( 0,  0), E( 6,  0), E( 5,  4), E(14,  8),
                                         E(30, 21), E(58, 43), E(79, 83), E( 0,  0)};
// Connected pawns
constexpr Score PAWN_CONNECTED_BONUS[8] = {E( 0,  0), E( 0,  0), E(14,  5), E( 8,  3),
                                           E(17, 10), E(36, 29), E(67, 64), E( 0,  0)};
// King-pawn tropism
constexpr int KING_TROPISM_VALUE = 18;

// Endgame win probability adjustment
constexpr int PAWN_ASYMMETRY_BONUS = 4;
constexpr int PAWN_COUNT_BONUS = 5;
constexpr int KING_OPPOSITION_DISTANCE_BONUS = 2;
constexpr int ENDGAME_BASE = -38;

// Scale factors for drawish endgames
constexpr int MAX_SCALE_FACTOR = 32;
constexpr int OPPOSITE_BISHOP_SCALING[2] = {13, 29};
constexpr int PAWNLESS_SCALING[4] = {1, 4, 8, 23};


#undef E

#endif
