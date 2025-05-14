#include <iostream>
#include "parameters.h"
#include "boards/precalculated/precalculated_small_boards.h"
#include "client/ClientMain.h"

/**
 * @brief Парсит аргументы командной строки и возвращает кортеж (port, archPath, timePerMove).
 * Если аргументов недостаточно, возвращается кортеж с port = -1.
 */
std::tuple<int, std::string, float> parseArguments(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                << " <port> <archPath> <timePerMove>\n";
        return {-1, "", -1.0f};
    }
    int port = std::atoi(argv[1]);
    std::string archPath = argv[2];
    float timePerMove = std::atof(argv[3]);
    return {port, archPath, timePerMove};
}

int main(int argc, char *argv[]) {
    srand(params::SEED);
    precalculateSmallBoardsArray();

    auto [port, archPath, timePerMove] = parseArguments(argc, argv);
    if (port == -1) return 1;

    ClientMain client(port, archPath, timePerMove);
    client.mainLoop();
    precalcBoardsFreeMem();
    return 0;
}
