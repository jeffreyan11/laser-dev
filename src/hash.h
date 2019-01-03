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

#ifndef __HASH_H__
#define __HASH_H__

#include "board.h"
#include "common.h"

constexpr uint8_t PV_NODE = 0;
constexpr uint8_t CUT_NODE = 1;
constexpr uint8_t ALL_NODE = 2;
constexpr uint8_t NO_NODE_INFO = 3;


// Struct storing all search information.
// Size: 8 bytes
struct HashData {
    int16_t score;
    Move move;
    uint8_t nodeType;
    uint8_t age;
    int8_t depth;
    int8_t _pad;

    HashData() = default;
    HashData(int _depth, Move _move, int _score, uint8_t _nodeType, uint8_t _age) {
        score = (int16_t) _score;
        move = _move;
        nodeType = _nodeType;
        age = _age;
        depth = (int8_t) _depth;
    }
};

// Struct storing hashed search information and corresponding hash key.
// Size: 16 bytes
struct HashEntry {
    uint64_t zobristKey;
    HashData data;

    HashEntry() { zobristKey = 0; }
    ~HashEntry() = default;

    void setEntry(Board &b, HashData _data) {
        zobristKey = b.getZobristKey();
        data = _data;
    }
};

// This contains each of the hash table entries, in a two-bucket system.
class HashNode {
public:
    HashEntry slot1;
    HashEntry slot2;

    HashNode() {}
    ~HashNode() {}
};

class Hash {
private:
    HashNode *table;
    uint64_t size;
    uint8_t age;

    void init(uint64_t MB);

public:
    Hash(uint64_t MB);
    Hash(const Hash &other) = delete;
    Hash& operator=(const Hash &other) = delete;
    ~Hash();

    void add(Board &b, HashData data, int depth);
    HashData *get(Board &b);

    uint64_t getSize();
    void setSize(uint64_t MB);

    void incrementAge();
    int getAge() const { return age; }

    void clear();
    int estimateHashfull();
};

#endif
