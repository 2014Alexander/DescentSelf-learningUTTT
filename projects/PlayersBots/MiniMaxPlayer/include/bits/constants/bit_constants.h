#pragma once

#include <cstdint>

namespace rights {

    constexpr uint64_t _1_BIT = 0x1;
    constexpr uint64_t _2_BITS = 0x3;
    constexpr uint64_t _3_BITS = 0x7;
    constexpr uint64_t _4_BITS = 0xF;
    constexpr uint64_t _5_BITS = 0x1F;
    constexpr uint64_t _6_BITS = 0x3F;
    constexpr uint64_t _7_BITS = 0x7F;
    constexpr uint64_t _8_BITS = 0xFF;
    constexpr uint64_t _9_BITS = 0x1FF;
    constexpr uint64_t _10_BITS = 0x3FF;
    constexpr uint64_t _11_BITS = 0x7FF;
    constexpr uint64_t _12_BITS = 0xFFF;
    constexpr uint64_t _13_BITS = 0x1FFF;
    constexpr uint64_t _14_BITS = 0x3FFF;
    constexpr uint64_t _15_BITS = 0x7FFF;
    constexpr uint64_t _16_BITS = 0xFFFF;
    constexpr uint64_t _17_BITS = 0x1FFFF;
    constexpr uint64_t _18_BITS = 0x3FFFF;
    constexpr uint64_t _19_BITS = 0x7FFFF;
    constexpr uint64_t _20_BITS = 0xFFFFF;
    constexpr uint64_t _21_BITS = 0x1FFFFF;
    constexpr uint64_t _22_BITS = 0x3FFFFF;
    constexpr uint64_t _23_BITS = 0x7FFFFF;
    constexpr uint64_t _24_BITS = 0xFFFFFF;
    constexpr uint64_t _25_BITS = 0x1FFFFFF;
    constexpr uint64_t _26_BITS = 0x3FFFFFF;
    constexpr uint64_t _27_BITS = 0x7FFFFFF;
    constexpr uint64_t _28_BITS = 0xFFFFFFF;
    constexpr uint64_t _29_BITS = 0x1FFFFFFF;
    constexpr uint64_t _30_BITS = 0x3FFFFFFF;
    constexpr uint64_t _31_BITS = 0x7FFFFFFF;
    constexpr uint64_t _32_BITS = 0xFFFFFFFF;
    constexpr uint64_t _33_BITS = 0x1FFFFFFFF;
    constexpr uint64_t _34_BITS = 0x3FFFFFFFF;
    constexpr uint64_t _35_BITS = 0x7FFFFFFFF;
    constexpr uint64_t _36_BITS = 0xFFFFFFFFF;
}
namespace board {
    namespace pos {
        constexpr uint64_t X_part = 0;
        constexpr uint64_t O_part = 9;
        constexpr uint64_t STATE = 18;
        constexpr uint64_t FREE_COUNT = 22;
        constexpr uint64_t FREE_CELLS = 26;
    }
    namespace mask {
        constexpr uint64_t X_part = rights::_9_BITS;
        constexpr uint64_t O_part = rights::_9_BITS;
        constexpr uint64_t OX_parts = rights::_18_BITS;
        constexpr uint64_t STATE = rights::_4_BITS;
        constexpr uint64_t FREE_COUNT = rights::_4_BITS;
        constexpr uint64_t FREE_CELL = rights::_4_BITS;
        constexpr uint64_t FREE_CELLS = rights::_36_BITS;
    }
}
namespace bigBoardArrays {
    constexpr int size = 12;
    constexpr int movesSize = 81;
    constexpr int bigState1Pos = 9;
    constexpr int bigState2Pos = 10;
    constexpr int hashKeyPos = 11;
}
namespace bigState1 {
    namespace pos {
        constexpr uint64_t X_part = 0;
        constexpr uint64_t O_part = 9;
        constexpr uint64_t ONGOING = 18;
    }
    namespace mask {
        constexpr uint64_t X_part = rights::_9_BITS;
        constexpr uint64_t O_part = rights::_9_BITS;
        constexpr uint64_t OX_part = rights::_18_BITS;
        constexpr uint64_t ONGOING = rights::_9_BITS;
    }
}
namespace bigState2 {
    namespace pos {
        constexpr uint64_t STATE = 0;
        constexpr uint64_t PLAYER = 4;
        constexpr uint64_t VALID_BOARDS_COUNT = 5;
        constexpr uint64_t VALID_BOARDS = 9;
    }
    namespace mask {
        constexpr uint64_t STATE = rights::_4_BITS;
        constexpr uint64_t PLAYER = rights::_1_BIT;
        constexpr uint64_t VALID_BOARDS_COUNT = rights::_4_BITS;
        constexpr uint64_t VALID_BOARDS = rights::_36_BITS;
    }
}
namespace stateCode {
    constexpr uint64_t X_WINS = 0b0001;
    constexpr uint64_t O_WINS = 0b0010;
    constexpr uint64_t ONGOING = 0b0100;
    constexpr uint64_t DRAW = 0b1000;
    namespace bitPos {
        constexpr uint64_t X_WINS = 0;
        constexpr uint64_t O_WINS = 1;
        constexpr uint64_t ONGOING = 2;
        constexpr uint64_t DRAW = 3;
    }
}
namespace cell {
    constexpr int X = 0;
    constexpr int O = 1;
    constexpr int EMPTY = 2;
}
namespace move {
    namespace mask {
        constexpr uint64_t cellIndex = rights::_4_BITS;
        constexpr uint64_t boardIndex = rights::_4_BITS;
    }
    namespace pos {
        constexpr uint64_t boardIndex = 4;
        constexpr uint64_t cellIndex = 0;
    }
}
