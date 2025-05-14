// ───────────────────────────────────────────────── main.cpp
#include <iostream>
#include <random>

#include "boards/precalculated/precalculated_small_boards.h"
#include "client/ClientMain.h"
#include "selfplay/evaluate/SmallBoardsEvaluator.h"

/**
 * @brief Парсит аргументы командной строки и возвращает пару (port, timePerMove).
 * Если аргументов недостаточно, возвращается {-1, -1.0f}.
 */
static std::pair<int, float> parseArguments(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <timePerMove>\n";
        return {-1, -1.0f};
    }
    int port = std::atoi(argv[1]);
    float timePerMove = std::atof(argv[2]);
    return {port, timePerMove};
}

int main(int argc, char *argv[]) {
    std::srand(std::random_device{}());

    precalculateSmallBoardsArray();
    SmallBoardsEvaluator::precalculate();
    // парсим порт и лимит времени
    auto [port, timePerMove] = parseArguments(argc, argv);
    if (port < 0 || timePerMove <= 0.0f) {
        return 1;
    }

    ClientMain client(port, timePerMove);
    client.mainLoop();

    precalcBoardsFreeMem();
    SmallBoardsEvaluator::freeMemory();
    return 0;
}
