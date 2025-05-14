// TournamentRunner.cpp
#include "tournament_system/tournament/TournamentRunner.h"
#include <iostream>
#include <array>


/* ------------------------------------------------------------------ */
TournamentRunner::TournamentRunner(std::string cfgPath)
    : pm(std::move(cfgPath))
      , provider(pm) {
}

// конструктор по умолчанию — просто делегируем
TournamentRunner::TournamentRunner()
    : TournamentRunner("config/general_testing_system_params.txt") {
    precalculateSmallBoardsArray();
}

TournamentRunner::~TournamentRunner() {
    precalcBoardsFreeMem();
}

void TournamentRunner::run() {
    auto pairs = provider.getPlayerPairs();
    int gamesPerPair = GeneralParams::pairGamesNum;

    constexpr std::array<uint8_t, 81> firstMoves = {
        0x44, 0x08, 0x26, 0x62, 0x80, 0x41, 0x43, 0x45, 0x47, 0x17, 0x35, 0x53, 0x71,
        0x11, 0x33, 0x55, 0x77, 0x01, 0x03, 0x21, 0x25, 0x63, 0x67, 0x85, 0x87, 0x05,
        0x07, 0x23, 0x27, 0x61, 0x65, 0x81, 0x83, 0x13, 0x15, 0x31, 0x37, 0x51, 0x57,
        0x73, 0x75, 0x40, 0x42, 0x46, 0x48, 0x02, 0x06, 0x20, 0x28, 0x60, 0x68, 0x82,
        0x86, 0x16, 0x18, 0x32, 0x38, 0x50, 0x56, 0x70, 0x72, 0x04, 0x24, 0x64, 0x84,
        0x00, 0x22, 0x66, 0x88, 0x10, 0x12, 0x30, 0x36, 0x52, 0x58, 0x76, 0x78, 0x14,
        0x34, 0x54, 0x74
    };

    std::cout << "=== TOURNAMENT START ===\n";
    int idx = 0;
    for (auto &pp: pairs) {
        std::cout << "\n-- Pair " << ++idx << ": " << pp.toString() << '\n';

        /* --- один раз поднимаем процессы --- */
        ClientWrapper botA, botB;
        botA.setPlayer(pp.first); // процесс #1
        botB.setPlayer(pp.second); // процесс #2

        for (int g = 0; g < gamesPerPair; ++g) {
            bool swap = g & 1; // во 2‑й,4‑й,… партии меняемся
            ClientWrapper &X = swap ? botB : botA;
            ClientWrapper &O = swap ? botA : botB;
            Referee ref(X, O);
            ref.playOneGame(firstMoves[g / 2]);
            std::cout << "   game " << g + 1 << '/' << gamesPerPair << " done\n";
        }

        /* --- закрываем процессы после всех игр пары --- */
        botA.closeProcess();
        botB.closeProcess();
    }
    std::cout << "\n=== TOURNAMENT FINISHED ===\n";
}
