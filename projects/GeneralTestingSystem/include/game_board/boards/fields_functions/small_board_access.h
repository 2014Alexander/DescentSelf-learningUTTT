#pragma once

#include <cstdint>
#include "game_board/bits/constants/bit_constants.h"

namespace boardGet {
    constexpr uint64_t Xpart(uint64_t board) {
        return (board >> board::pos::X_part) & board::mask::X_part;
    }

    constexpr uint64_t Opart(uint64_t board) {
        return (board >> board::pos::O_part) & board::mask::O_part;
    }

    constexpr uint64_t OXpartsMeged(uint64_t board) {
        return ((board >> board::pos::X_part) & board::mask::X_part) |
               ((board >> board::pos::O_part) & board::mask::O_part);
    }

    constexpr uint64_t state(uint64_t board) {
        return (board >> board::pos::STATE) & board::mask::STATE;
    }

    constexpr uint64_t freeCount(uint64_t board) {
        return (board >> board::pos::FREE_COUNT) & board::mask::FREE_COUNT;
    }

    constexpr uint64_t freeCells(uint64_t board) {
        return (board >> board::pos::FREE_CELLS) & board::mask::FREE_CELLS;
    }

    constexpr uint64_t code(uint64_t board) {
        return board & (board::mask::OX_parts);
    }

}
