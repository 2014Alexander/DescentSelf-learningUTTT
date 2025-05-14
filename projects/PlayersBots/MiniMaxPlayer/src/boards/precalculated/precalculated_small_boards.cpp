// precalculated_small_boards.cpp
#include "boards/precalculated/precalculated_small_boards.h"
#include "bits/constants/bit_constants.h"
#include "boards/fields_functions/small_board_access.h"
#include <bit>
#include <cstring>

constexpr uint16_t WIN_MASKS[8] = {
    0b000000111, // Horizontal 1
    0b000111000, // Horizontal 2
    0b111000000, // Horizontal 3
    0b001001001, // Vertical 1
    0b010010010, // Vertical 2
    0b100100100, // Vertical 3
    0b100010001, // Diagonal 1
    0b001010100 // Diagonal 2
};

void precalculateZerosArray() {
    for (uint16_t i = 0; i < 512; ++i) {
        // 512 = 2^9
        zerosCountedArray[i] = 9 - std::popcount(i);
    }
}

void precalculateSmallBoardsArray() {
    std::memset(boardsInfoArray, 0, TOTAL_BOARDS * sizeof(uint64_t));

    for (uint64_t code = 0; code < TOTAL_BOARDS; ++code) {
        uint64_t X_mask = boardGet::Xpart(code);
        uint64_t O_mask = boardGet::Opart(code);
        if (X_mask & O_mask) {
            continue;
        }
        uint64_t x_count = 0;
        uint64_t o_count = 0;
        uint64_t boardState = 0;

        uint64_t combined_mask = X_mask | O_mask;

        for (uint64_t pos = 0; pos < 9; ++pos) {
            uint64_t bit = (rights::_1_BIT << pos);
            if (X_mask & bit) {
                ++x_count;
            } else if (O_mask & bit) {
                ++o_count;
            }
        }

        bool x_wins = false;
        for (const auto &mask: WIN_MASKS) {
            if ((X_mask & mask) == mask) {
                x_wins = true;
                break;
            }
        }

        bool o_wins = false;
        for (const auto &mask: WIN_MASKS) {
            if ((O_mask & mask) == mask) {
                o_wins = true;
                break;
            }
        }

        if (x_wins && o_wins) {
            continue; // Niedozwolony stan
        }
        if (x_wins) {
            boardState = stateCode::X_WINS;
        } else if (o_wins) {
            boardState = stateCode::O_WINS;
        } else if (x_count + o_count == 9) {
            boardState = stateCode::DRAW;
        } else {
            boardState = stateCode::ONGOING;
        }

        // Liczenie wolnych komórek i ich numerów
        uint64_t free_cells = 0;
        uint64_t free_count = 0;
        if (boardState == stateCode::ONGOING) {
            for (uint64_t pos = 0; pos < 9; ++pos) {
                uint64_t bit = (rights::_1_BIT << pos);
                if ((combined_mask & bit) == 0) {
                    free_cells |= (pos << (free_count * 4));
                    ++free_count;
                }
            }
        }
        X_mask <<= board::pos::X_part;
        O_mask <<= board::pos::O_part;
        boardState <<= board::pos::STATE;
        free_count <<= board::pos::FREE_COUNT;
        free_cells <<= board::pos::FREE_CELLS;

        boardsInfoArray[code] = X_mask | O_mask | boardState | free_count | free_cells;
    }
    precalculateZerosArray();
}

void precalcBoardsFreeMem() {
    if (boardsInfoArray != nullptr) {
        delete[] boardsInfoArray;
        boardsInfoArray = nullptr;
        delete[] zerosCountedArray;
    }
}
