// ────────────────────────────────────────── Player.h
#pragma once
#include <cstdint>
#include "big_board/BigBoard.h"
#include "selfplay/NegamaxAgent.h"

/**
 * Игрок, использующий NegamaxAgent.
 */
class Player {
public:
    explicit Player(float timePerMoveSec)
        : timeLimit(timePerMoveSec) {
        bigBoard.stateInit();
    }

    /* ----- вызовы из ClientMain ----- */

    /// применяем ход соперника
    inline void applyLocalMove(uint8_t move) {
        bigBoard.applyMove(move);
    }

    /// выбираем собственный ход
    uint8_t makeNextMove() {
        uint8_t best = negamax.search(&bigBoard, timeLimit);
        bigBoard.applyMove(best); // фиксируем его локально
        std::cout << "[Minimax Player] Depth Reached: " << negamax.depthReached() << std::endl;
        std::cout << "[Minimax Player] Score: " << negamax.bestScore(&bigBoard) << std::endl;
        return best;
    }

private:
    BigBoard bigBoard;
    NegamaxAgent negamax;
    float timeLimit;
};
