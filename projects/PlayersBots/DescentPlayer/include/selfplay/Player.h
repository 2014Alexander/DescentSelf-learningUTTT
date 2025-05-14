#pragma once

#include <cstdlib>   // rand()
#include <chrono>
#include <algorithm> // std::sort

#include "Descent.h"
#include "parameters.h"
#include "big_board/BigBoard.h"
#include "structures/Map_T.h"
#include "structures/Set_S.h"

class Player {
private:
    BigBoard bigBoard;
    Set_S setS;
    Map_T mapV;
    Descent descent;
    SharedMemory &sharedMemory;
    float timePerMove;

public:
    Player(SharedMemory &shm, float timePerMove)
        : descent(setS, mapV, shm)
          , sharedMemory(shm)
          , timePerMove(timePerMove) {
        bigBoard.stateInit();
    }

    void applyLocalMove(uint8_t moveByte) {
        bigBoard.applyMove(moveByte);
    }

    uint8_t makeNextMove() {
        // (1) Запускаем Descent, чтобы заполнить оценки
        descent.descent(&bigBoard, timePerMove);

        // (2) Выбираем ход (selectMoveOrdinal будет вставлен вами)
        uint8_t chosenMove = selectMoveOrdinal(&bigBoard, params::ORDINAL_ACTION_RATIO);

        // (3) Применяем ход локально
        bigBoard.applyMove(chosenMove);

        // (4) Возвращаем сделанный ход
        return chosenMove;
    }

private:
    /**
     * Реализует Ordinal action distribution:
     *  - сортируем ходы по убыванию/возрастанию в зависимости от игрока;
     *  - идём от лучшего к худшему, на j-м шаге бросаем случай [0..1];
     *  - c вероятностью p берём текущий ход, иначе идём к j+1;
     *  - если дошли до конца — берём последний.
     *  - ratio = 0:  равномерное распределение по всем 𝑛 − 𝑗 оставшимся вариантам.
     *  - ratio = 1:  выбираем первый (самый лучший) ход
     */
    uint8_t selectMoveOrdinal(BigBoard *board, float ratio) {
        uint8_t *moves = board->getValidMoves();
        int movesCount = moves[0];
        if (movesCount == 0) {
            return 0;
        }

        bool firstPlayer = (board->getCurrentPlayer() == cell::X);

        MoveVal mv[81];

        for (int i = 0; i < movesCount; ++i) {
            uint8_t m = moves[i + 1];
            mv[i].move = m;
            mv[i].val = mapV(board, m);
        }

        // Сортируем массив с использованием оптимизированных компараторов
        if (firstPlayer) {
            std::sort(mv, mv + movesCount, compare_desc);
        } else {
            std::sort(mv, mv + movesCount, compare_asc);
        }

        // Алгоритм Ordinal:
        // На j-м шаге p = ratio * ((n-j-1)/(n-j)) + (1/(n-j))
        int n = movesCount;
        for (int j = 0; j < n; j++) {
            float p = ratio * (float(n - j - 1) / float(n - j))
                      + (1.0f / float(n - j));

            float r = float(rand()) / float(RAND_MAX);
            if (r < p) {
                return mv[j].move;
            }
        }

        // Если все "отвергли" - берём последний
        return mv[n - 1].move;
    }

    struct MoveVal {
        uint8_t move;
        float val;
    };

    static inline bool compare_desc(const MoveVal &a, const MoveVal &b) noexcept {
        return a.val > b.val;
    }

    static inline bool compare_asc(const MoveVal &a, const MoveVal &b) noexcept {
        return a.val < b.val;
    }
};
