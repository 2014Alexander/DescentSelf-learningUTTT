// big_board_get_set_do.h
#pragma once

#include <cstdint>
#include "game_board/boards/fields_functions/small_board_access.h"
#include "game_board/bits/constants/bit_constants.h"

namespace BigBoardGet {
    inline uint64_t state(uint64_t bigState2) {
        return (bigState2 >> bigState2::pos::STATE) & bigState2::mask::STATE;
    }

    inline uint64_t player(uint64_t bigState2) {
        return (bigState2 >> bigState2::pos::PLAYER) & bigState2::mask::PLAYER;
    }

    inline uint64_t validBoardsCount(uint64_t bigState2) {
        return (bigState2 >> bigState2::pos::VALID_BOARDS_COUNT) & bigState2::mask::VALID_BOARDS_COUNT;
    }

    inline uint64_t validBoards(uint64_t bigState2) {
        return (bigState2 >> bigState2::pos::VALID_BOARDS) & bigState2::mask::VALID_BOARDS;
    }

    inline uint64_t layerOngoing(uint64_t bigState1) {
        return (bigState1 >> bigState1::pos::ONGOING) & bigState1::mask::ONGOING;
    }

    inline uint64_t layerWinsX(uint64_t bigState1) {
        return (bigState1 >> bigState1::pos::X_part) & bigState1::mask::X_part;
    }

    inline uint64_t layerWinsO(uint64_t bigState1) {
        return (bigState1 >> bigState1::pos::O_part) & bigState1::mask::O_part;
    }


}
namespace BigBoardDo {
    inline void cleanValidBoards(uint64_t & bigState2) {
        constexpr uint64_t validBoardsMask = bigState2::mask::VALID_BOARDS << bigState2::pos::VALID_BOARDS;
        bigState2 &= ~validBoardsMask;
    }

    inline void invertPlayer(uint64_t &bigState2) {
        bigState2 ^= rights::_1_BIT << bigState2::pos::PLAYER;
    }

}
namespace BigBoardSet {

    inline void validBoardsCount(uint64_t &bigState2, uint64_t count) {
        bigState2 &= ~(bigState2::mask::VALID_BOARDS_COUNT << bigState2::pos::VALID_BOARDS_COUNT);
        bigState2 |= count << bigState2::pos::VALID_BOARDS_COUNT;
    }

    inline void validBoards(uint64_t &bigState2, uint64_t validBoards) {
        BigBoardDo::cleanValidBoards(bigState2);
        bigState2 |= validBoards << bigState2::pos::VALID_BOARDS;
    }
}
