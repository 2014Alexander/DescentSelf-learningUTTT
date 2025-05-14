// SmallBoardsEvaluator.h
// ───────────────────────────────────────────────────── SmallBoardsEvaluator.h
#pragma once
#include <cstdint>
#include "boards/precalculated/precalculated_small_boards.h" // boardsInfoArray, boardGet

class SmallBoardsEvaluator {
public:
    /// Вызывается один раз после precalculateSmallBoardsArray()
    static void precalculate();

    /// O(1) доступ к оценке (X-перспектива, >0 лучше X)
    static inline int getBoardEvaluation(uint64_t boardBits) {
        return evalArray[ boardGet::code(boardBits) ];
    }

    /// Освободить память (опционально)
    static void freeMemory();

private:
    static constexpr int TOTAL = TOTAL_BOARDS;    // 262 144
    static inline int16_t* evalArray = nullptr;    // 2 B × TOTAL = 512 KB

    // ----- веса из Python-эталона -----
    static constexpr int WIN_VAL  = 1000;
    static constexpr int W_FORKS  = 120;
    static constexpr int W_OPEN2  = 15;
    static constexpr int W_OPEN1  = 1;
    static constexpr int W_CENTER = 6;
    static constexpr int W_CORNER = 3;
    static constexpr int W_EDGE   = 1;

    // восемь выигрышных линий 3×3, битовые маски по 9 клеткам
    static constexpr uint16_t LINES[8] = {
        0b000000111, 0b000111000, 0b111000000,
        0b001001001, 0b010010010, 0b100100100,
        0b100010001, 0b001010100
    };

    // для каждой из 9 клеток – индексы выигрышных линий, в которых она участвует.
    // Последний элемент — sentinel 0xFF.
    static constexpr uint8_t CELL_LINES[9][5] = {
        {0, 3, 6, 0xFF, 0xFF}, // клетка 0
        {0, 4, 0xFF,0xFF,0xFF},// клетка 1
        {0, 5, 7, 0xFF, 0xFF}, // клетка 2
        {1, 3, 0xFF,0xFF,0xFF},// клетка 3
        {1, 4, 6, 7,   0xFF},  // клетка 4 (центр)
        {1, 5, 0xFF,0xFF,0xFF},// клетка 5
        {2, 3, 7, 0xFF, 0xFF}, // клетка 6
        {2, 4, 0xFF,0xFF,0xFF},// клетка 7
        {2, 5, 6, 0xFF, 0xFF}  // клетка 8
    };
};

