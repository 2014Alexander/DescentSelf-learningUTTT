// SmallBoardsEvaluator.cpp
// ───────────────────────────────────────────────────── SmallBoardsEvaluator.cpp
#include "selfplay/evaluate/SmallBoardsEvaluator.h"

#include "bits/constants/bit_constants.h"
#include "boards/fields_functions/small_board_access.h"
#include <cmath>  // std::round

void SmallBoardsEvaluator::precalculate() {
    if (evalArray != nullptr) return; // уже сделано
    evalArray = new int16_t[TOTAL];

    for (uint32_t code = 0; code < TOTAL; ++code) {
        uint64_t info = boardsInfoArray[code]; // =0 для нелегальных кодов
        uint64_t Xmask = (info >> board::pos::X_part) & board::mask::X_part;
        uint64_t Omask = (info >> board::pos::O_part) & board::mask::O_part;
        uint64_t state = (info >> board::pos::STATE) & board::mask::STATE;

        // --- 0) нелегальные позиции пропускаем ---
        if (state == 0) {
            evalArray[code] = 0;
            continue;
        }

        // --- 1) терминальные позиции ---
        if (state == stateCode::X_WINS) {
            evalArray[code] = WIN_VAL;
            continue;
        }
        if (state == stateCode::O_WINS) {
            evalArray[code] = -WIN_VAL;
            continue;
        }
        if (state == stateCode::DRAW) {
            evalArray[code] = 0;
            continue;
        }

        // --- 2) open-линиии ---
        int open2X = 0, open1X = 0, open2O = 0, open1O = 0;
        for (uint16_t m: LINES) {
            int nX = std::popcount(static_cast<uint16_t>(Xmask & m));
            int nO = std::popcount(static_cast<uint16_t>(Omask & m));
            if (nO == 0) {
                if (nX == 2) ++open2X;
                else if (nX == 1) ++open1X;
            }
            if (nX == 0) {
                if (nO == 2) ++open2O;
                else if (nO == 1) ++open1O;
            }
        }

        // --- 3) forks (ячейка в ≥2 open2-линии) ---
        int forksX = 0, forksO = 0;
        uint16_t empty = static_cast<uint16_t>(~(Xmask | Omask)) & rights::_9_BITS;
        for (int c = 0; c < 9; ++c) {
            if ((empty & (1u << c)) == 0) continue;
            int cntX = 0, cntO = 0;
            for (int li = 0; li < 5 && CELL_LINES[c][li] != 0xFF; ++li) {
                uint16_t m = LINES[CELL_LINES[c][li]];
                if ((m & Omask) == 0 && std::popcount(Xmask & m) == 2) ++cntX;
                if ((m & Xmask) == 0 && std::popcount(Omask & m) == 2) ++cntO;
            }
            if (cntX >= 2) ++forksX;
            if (cntO >= 2) ++forksO;
        }

        // --- 4) позиционные фичи ---
        int centerX = (Xmask >> 4) & 1;
        int centerO = (Omask >> 4) & 1;
        int cornersX = std::popcount(static_cast<uint16_t>(Xmask & 0b100010001));
        int cornersO = std::popcount(static_cast<uint16_t>(Omask & 0b100010001));
        int edgesX = std::popcount(static_cast<uint16_t>(Xmask & 0b010101010));
        int edgesO = std::popcount(static_cast<uint16_t>(Omask & 0b010101010));

        // --- 5) тактический и позиционный дифференциалы ---
        int tactDiff = W_FORKS * (forksX - forksO)
                       + W_OPEN2 * (open2X - open2O)
                       + W_OPEN1 * (open1X - open1O);

        int posDiff = W_CENTER * (centerX - centerO)
                      + W_CORNER * (cornersX - cornersO)
                      + W_EDGE * (edgesX - edgesO);

        // --- 6) фазовый коэффициент для позиционной части ---
        double freeCount = double((info >> board::pos::FREE_COUNT) & board::mask::FREE_COUNT);
        double stage = freeCount / 9.0; // 1.0…0.0
        double posK = stage * stage;

        double score = tactDiff + posK * posDiff;
        evalArray[code] = static_cast<int16_t>(std::round(score));
    }
}

void SmallBoardsEvaluator::freeMemory() {
    delete[] evalArray;
    evalArray = nullptr;
}
