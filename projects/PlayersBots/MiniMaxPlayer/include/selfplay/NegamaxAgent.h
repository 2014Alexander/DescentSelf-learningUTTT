// ─────────────────────────────────────────────────── NegamaxAgent.h
#pragma once

#include <chrono>
#include <algorithm>
#include <vector>
#include "big_board/BigBoard.h"
#include "selfplay/evaluate/BigBoardsEvaluator.h"   // <-- новая зависимость
/**
 * Простая итеративно-углубляемая αβ-негамакс.
 * Оценка нетерминала = BigBoardsEvaluator::evaluate(bigBoard).
 */
class NegamaxAgent {
public:
    NegamaxAgent()
        : timeout_(false),
          reachedDepth_(0),
          bestScore_(0),
          bestOverall_(0) {
    }

    /** Поиск лучшего хода за не более timeLimitSec секунд. */
    uint8_t search(BigBoard *board, double timeLimitSec) {
        if (board->isGameOver())
            return 0;

        /* ---- собираем корневые ходы ---- */
        uint8_t *rootMovesArr = board->getValidMoves();
        int movesCount = rootMovesArr[0];

        struct MoveScore {
            uint8_t move;
            long score;
        };
        std::vector<MoveScore> rootMoves;
        rootMoves.reserve(movesCount);
        for (int i = 0; i < movesCount; ++i)
            rootMoves.push_back({rootMovesArr[i + 1], 0L});

        reachedDepth_ = 0;
        bestScore_ = 0;
        bestOverall_ = 0;
        timeout_ = false;
        nodesSinceLastTimeCheck_ = 0;
        end_ = std::chrono::steady_clock::now()
               + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                   std::chrono::duration<double>(timeLimitSec)
               );
        /* ---- итеративное углубление ---- */
        for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
            if (depth > 1)
                std::sort(rootMoves.begin(), rootMoves.end(),
                          [](auto &a, auto &b) {
                              return a.score > b.score;
                          });

            long alpha = -INF, beta = INF;
            long bestScoreDepth = -INF;
            uint8_t bestMoveDepth = 0;

            for (auto &ms: rootMoves) {
                BigBoard child = *board;
                child.applyMove(ms.move);

                long score = -alphaBeta(&child, depth - 1, -beta, -alpha);
                ms.score = score;

                if (timeout_) break;

                if (score > bestScoreDepth) {
                    bestScoreDepth = score;
                    bestMoveDepth = ms.move;
                }
                alpha = std::max(alpha, score);
            }

            if (timeout_) break;

            reachedDepth_ = depth;
            bestScore_ = bestScoreDepth;
            bestOverall_ = bestMoveDepth;
        }

        return bestOverall_;
    }

    int depthReached() const {
        return reachedDepth_;
    }

    long bestScore(BigBoard *board) const {
        return (board->getCurrentPlayer() == cell::X) ? -bestScore_ : bestScore_;
    }

private:
    /* ---------- рекурсивный αβ-негамакс ---------- */
    long alphaBeta(BigBoard *board, int depth, long alpha, long beta) {
        if (checkTimeout())
            return 0;

        if (depth == 0 || board->isGameOver())
            return evaluate(board);

        uint8_t *movesArr = board->getValidMoves();
        int movesCount = movesArr[0];

        for (int i = 1; i <= movesCount; ++i) {
            BigBoard child = *board;
            child.applyMove(movesArr[i]);

            long score = -alphaBeta(&child, depth - 1, -beta, -alpha);
            if (timeout_) return 0;

            if (score >= beta) // fail-soft β-cut
                return score;

            alpha = std::max(alpha, score);
        }
        return alpha;
    }

    /* ---------- эвристика ---------- */
    inline long evaluate(BigBoard *board) {
        long v = BigBoardsEvaluator::evaluate(*board);
        return (board->getCurrentPlayer() == cell::X) ? v : -v;
    }

    inline bool checkTimeout() {
        if (timeout_) return true;
        if (++nodesSinceLastTimeCheck_ >= CHECK_PERIOD) {
            nodesSinceLastTimeCheck_ = 0;
            if (std::chrono::steady_clock::now() >= end_) {
                timeout_ = true;
            }
        }
        return timeout_;
    }


    /* ---------- константы ---------- */
    static constexpr long INF = 1'000'000'000L;
    static constexpr int MAX_DEPTH = 81;

    /* ---------- поля состояния поиска ---------- */
    int reachedDepth_;
    long bestScore_;
    uint8_t bestOverall_;
    /* ---------- таймаут ---------- */
    bool timeout_;
    std::chrono::steady_clock::time_point end_;
    int nodesSinceLastTimeCheck_ = 0;
    static constexpr int CHECK_PERIOD = 128;
};
