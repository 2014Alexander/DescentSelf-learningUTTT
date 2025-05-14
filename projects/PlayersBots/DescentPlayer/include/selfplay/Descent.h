// Descent.h
#pragma once

#include <chrono>

#include "BatchEvaluator.h"
#include "parameters.h"
#include "structures/Set_S.h"
#include "structures/Map_T.h"
#include "big_board/BigBoard.h"
#include "utils/Counter.h"


class Descent {
private:
    long evaluatedStateCount;

public:
    /**
     * @param s - ссылка на множество состояний (Set_S), в котором храним посещённые BigBoard
     * @param v - ссылка на Map_T, где храним v(s) и v'(s,a)
     */
    Descent(Set_S &s, Map_T &v, SharedMemory &shm)
        : S(s), V(v), counter(params::DESCENT_ITERATION_COUNT), batchEvaluator(shm, v) {
    }

    /**
     * Запуск descent, повторяя многократно descentIteration, пока не выйдет лимит времени.
     *
     * @param board          - текущее состояние
     * @param moveTimeLimit  - лимит времени (в секундах) на совокупность итераций
     */
    // void descent(BigBoard *board, float moveTimeLimit) {
    //     resetTimer();
    //     while (!isTimeExceeded(moveTimeLimit)) {
    //         descentIteration(board);
    //     }
    // }

    void descent(BigBoard *board, float moveTimeLimit) {
        // while (!counter.isCountExceeded()) {
        //     descentIteration(board);
        // }
        resetTimer();
        int iterCount = 0;
        evaluatedStateCount = 0;
        while (!isTimeExceeded(moveTimeLimit)) {
            descentIteration(board);
            ++iterCount;
        }
        std::cout << "\niterations count = " << iterCount << std::endl;
        std::cout << "States NN evaluated = " << evaluatedStateCount << std::endl;
    }

    /**
    *Function descent_iteration(s, S, T, fθ, ft)
    *    if terminal(s) then
    *        S ← S ∪ {s}
    *        v(s) ← ft(s)
    *    else
    *        if s ∉ S then
    *            S ← S ∪ {s}
    *            foreach a ∈ actions(s) do
    *                if terminal(a(s)) then
    *                    S ← S ∪ {a(s)}
    *                    v′(s, a) ← ft(a(s))
    *                    v(a(s)) ← v′(s, a)
    *                else
    *                    v′(s, a) ← fθ(a(s))
    *        ab ← best_action(s)
    *        v′(s, ab) ← descent_iteration(ab(s), S, T, fθ, ft)
    *        ab ← best_action(s)
    *        v(s) ← v′(s, ab)
    *    return v(s)
     */
    float descentIteration(BigBoard *state) {
        // 1) Проверка на терминальность
        if (state->isGameOver()) {
            float score = state->getTerminalScore();
            if (!S.contains(state))
                S.add(state); // S ← S ∪ {s}
            V(state) = score; // v(s) ← ft(s)
            return score;
        }

        // 2) Если s не в S, инициализируем v'(s,a)
        if (!S.contains(state)) {
            S.add(state);

            uint8_t *moves = state->getValidMoves(); // все действия

            int movesCount = moves[0];
            for (int i = 1; i <= movesCount; ++i) {
                uint8_t move = moves[i]; //foreach a ∈ actions(s)

                BigBoard stateAfterMove = *state; // child = a(s)
                stateAfterMove.applyMove(move);

                if (stateAfterMove.isGameOver()) {
                    S.add(&stateAfterMove); // S ← S ∪ {a(s)}
                    float termVal = stateAfterMove.getTerminalScore();
                    V(state, move) = termVal; // v′(s,a) ← ft(a(s))
                    V(&stateAfterMove) = termVal; // v(a(s)) ← v′(s,a)
                } else {
                    batchEvaluator.addNonTerminalState(stateAfterMove, move); //v′(s, a) ← fθ(a(s))
                    evaluatedStateCount++;
                }
            }
            batchEvaluator.evaluateAllNonTerminalChildStates(state);
        }

        uint8_t bestMove = bestActionOf(state); // ab ← best_action(s)
        {
            BigBoard stateAfterBestMove = *state;
            stateAfterBestMove.applyMove(bestMove);
            // v′(s, ab) ← descent_iteration(ab(s))
            V(state, bestMove) = descentIteration(&stateAfterBestMove);
        }

        uint8_t finalAction = bestActionOf(state); // ab ← best_action(s) (повторный вызов)
        float finalVal = V(state, finalAction); // v(s) ← v′(s, ab)
        V(state) = finalVal;

        return finalVal; // return v(s)
    }

    /**
     * Поиск лучшего действия (best_action) в зависимости от игрока.
     * Если текущий игрок X=0, то берём argmax,
     * если O=1, то argmin.
     */
    inline uint8_t bestActionOf(BigBoard *state) {
        // Сразу определим, кто игрок (X=0 => maximize, O=1 => minimize)
        const bool isFirstPlayer = (state->getCurrentPlayer() == cell::X);

        const uint8_t *moves = state->getValidMoves();
        const int movesCount = moves[0];

        // Адрес массива действий (пропускаем moves[0], где хранится movesCount)
        const uint8_t *actions = moves + 1;

        if (isFirstPlayer) {
            // Максимизируем v'(s,a)
            float bestVal = -1e9f;
            uint8_t bestAction = actions[0];

            for (int i = 0; i < movesCount; ++i) {
                uint8_t action = actions[i];
                float val = V(state, action); // v'(s,a)
                if (val > bestVal) {
                    bestVal = val;
                    bestAction = action;
                }
            }
            return bestAction;
        } else {
            // Минимизируем v'(s,a)
            float bestVal = 1e9f;
            uint8_t bestAction = actions[0];

            for (int i = 0; i < movesCount; ++i) {
                uint8_t a = actions[i];
                float val = V(state, a); // v'(s,a)
                if (val < bestVal) {
                    bestVal = val;
                    bestAction = a;
                }
            }
            return bestAction;
        }
    }

private:
    Set_S &S; ///< Хранилище уникальных состояний
    Map_T &V; ///< Карта оценок: v(s) и v'(s,a)
    Counter counter;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    /**
     * @brief Экземпляр BatchEvaluator, позволяющий батч-оценку нетерминальных состояний
     */
    BatchEvaluator batchEvaluator;
    /**
     * Сбрасывает таймер при начале descent.
     */
    void resetTimer() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    /**
     * Проверяет, истекло ли moveTimeLimit секунд с момента resetTimer().
     */
    bool isTimeExceeded(float moveTimeLimit) const {
        using namespace std::chrono;
        float elapsed = duration<float>(high_resolution_clock::now() - startTime).count();
        return (elapsed >= moveTimeLimit);
    }
};
