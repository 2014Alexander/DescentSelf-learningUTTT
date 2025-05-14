// precalculated_small_boards.h
#pragma once

#include <cstdint>
#include "game_board/boards/fields_functions/small_board_access.h"


constexpr int TOTAL_BOARDS = 262144;
alignas(64) inline uint64_t *boardsInfoArray = new uint64_t[TOTAL_BOARDS];
alignas(64) inline int *zerosCountedArray = new int[512];

inline uint64_t getBoardInfo(uint64_t board) noexcept {
    return boardsInfoArray[boardGet::code(board)];
}

void precalculateSmallBoardsArray(); // Функция инициализации массива
void precalcBoardsFreeMem();
