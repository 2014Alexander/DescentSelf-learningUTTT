// Referee.h
#pragma once
#include <cstdint>

#include "tournament_system/client/ClientWrapper.h"
#include "tournament_system/referee/RefereeBoard.h"

/// Мини‑судья: играет **одну** партию, не создаёт/не закрывает процессов.
/// Процессы уже запущены снаружи и передаются по ссылке.
class Referee {
public:
    ClientWrapper *clientX; ///< кто ходит крестиками в данной партии
    ClientWrapper *clientO; ///< ноликами
    RefereeBoard board;

    Referee(ClientWrapper &x, ClientWrapper &o);

    /// сыграть одну партию; первый ход делает X (`firstMoveX`)
    void playOneGame(uint8_t firstMoveX);
};
