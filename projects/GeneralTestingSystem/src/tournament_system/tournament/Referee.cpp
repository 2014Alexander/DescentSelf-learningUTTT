// Referee.cpp
#include "tournament_system/tournament/Referee.h"

Referee::Referee(ClientWrapper &x, ClientWrapper &o)
    : clientX(&x)
      , clientO(&o)
      , board(*this) // RefereeBoard хранит ссылку на Referee
{
}

/* ------------------------------------------------------------ */

void Referee::playOneGame(uint8_t first) {
    board.gameReset();
    clientX->gameReset();
    clientO->gameReset();

    /* первый ход */
    clientX->applyMove(first);
    clientO->applyMove(first);
    board.applyMove(first);

    /* основной цикл */
    while (true) {
        /* ход O */
        uint8_t mO = clientO->makeMove();
        board.applyMove(mO);
        if (board.isGameOver()) break;
        clientX->applyMove(mO);

        /* ход X */
        uint8_t mX = clientX->makeMove();
        board.applyMove(mX);
        if (board.isGameOver()) break;
        clientO->applyMove(mX);
    }
}
