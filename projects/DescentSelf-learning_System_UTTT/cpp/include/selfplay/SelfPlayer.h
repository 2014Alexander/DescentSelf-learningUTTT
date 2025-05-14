#pragma once

#include <cstdlib>   // rand()
#include <chrono>
#include <algorithm> // std::sort
#include <iostream>

#include "structures/ReplayBuffer.h"
#include "structures/Set_S.h"
#include "structures/Map_T.h"
#include "big_board/BigBoard.h"
#include "Descent.h" // Предполагаем, что этот класс реализует descent(board, moveTimeLimit)
#include "boards/utils/big_board_renderer.h"

class SelfPlayer {
public:
    explicit SelfPlayer(ReplayBuffer &replayBuffer, SharedMemory &shm)
        : replayBuffer(replayBuffer),
          descentLogic(S, V, shm) // Передаём ссылки на S и V в конструктор Descent
    {
    }

    /**
     * Запускает процесс самоигры, пока в буфере не накопится достаточно новых данных.
     * @param moveTimeLimit - время (в сек) для Descent
     */
    void runSelfPlay() {
        while (!replayBuffer.isEnoughNewData()) {
            playSingleGame();
            std::cout << "---Before Added---" << std::endl;
            std::cout << "New Added: " << replayBuffer.newAddedCount << std::endl;
            std::cout << "Buffer Size: " << replayBuffer.bufferSize() << std::endl;
            std::cout << "S.size: " << S.size << std::endl;
            replayBuffer.moveAll(S, V); // После партии переносим все (s, v(s)) из S в буфер,
            std::cout << "---After Added---" << std::endl;
            std::cout << "New Added: " << replayBuffer.newAddedCount << std::endl;
            std::cout << "Buffer Size: " << replayBuffer.bufferSize() << std::endl;
            std::cout << "S.size: " << S.size << std::endl;
        }
    }

private:
    ReplayBuffer &replayBuffer; ///< Буфер для (состояние, значение)
    Map_T V; ///< Хранит v(s) и v'(s,a)
    Set_S S; ///< Хранит множество уникальных состояний
    Descent descentLogic; ///< Алгоритм Descent, работающий с S и V
    int moveNum = 0;
    /**
     * Одна партия самоигры: до терминального состояния.
     * В каждом ходу вызываем Descent, а затем выбираем ход по Ordinal distribution.
     */
    void playSingleGame() {
        BigBoard *board = new BigBoard();
        moveNum = 0;
        while (!board->isGameOver()) {
            descentLogic.descent(board, params::MOVE_TIME_LIMIT); // S, T ← descent(s, S, T, fθ, ft)
            uint8_t action = selectMoveOrdinal(board, params::ORDINAL_ACTION_RATIO); //a ← action_selection(s, S, T)
            board->applyMove(action); //s ← a(s)
            drawBigBoard(*board);
            std::cout << "S.size = " << S.size << std::endl;
            std::cout << "Move Num: " << moveNum << std::endl;
            moveNum++;
        }
        delete board;
    }

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
        if (moveNum == 0) {
            ratio = 0.0;
        }
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
            mv[i].val = V(board, m);
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
