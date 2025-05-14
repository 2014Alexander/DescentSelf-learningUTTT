#pragma once

#include "tournament_system/setup/players_programs/parameters/GeneralParams.h"
#include "tournament_system/referee/ResultsLogger.h"
#include "game_board/big_board/BigBoard.h"
#include "game_board/boards/utils/big_board_renderer.h"

class Referee;

/// Ведёт состояние партии и логирует её результат.
class RefereeBoard {
public:
    explicit RefereeBoard(Referee &r)
        : board(new BigBoard()),
          ref(r),
          logger(GeneralParams::resultsFilePath, r) {
        board->stateInit();
    }

    ~RefereeBoard() {
        delete board;
    }

    void gameReset() {
        board->stateInit();
        moveNum = 0;
        lastMove = static_cast<uint8_t>(-1);
        drawBigBoard(*board, *this);
    }

    void applyMove(uint8_t a) {
        board->applyMove(a);
        ++moveNum;
        lastMove = a;
        drawBigBoard(*board, *this);
    }

    bool isGameOver() {
        if (!board->isGameOver()) return false;
        float s = board->getTerminalScore();
        logger.addResult(s > 0.f ? 1 : (s < 0.f ? -1 : 0));
        return true;
    }

    BigBoard *board;
    Referee &ref; // ref.clientX / clientO теперь указатели
    ResultsLogger logger;
    int moveNum = 0;
    uint8_t lastMove = static_cast<uint8_t>(-1);
};
