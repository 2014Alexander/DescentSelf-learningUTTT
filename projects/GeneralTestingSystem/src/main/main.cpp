// src/main.cpp
#include <iostream>
#include <winsock2.h>
#include "tournament_system/tournament/TournamentRunner.h"

int main() {
    // std::cout << std::unitbuf;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[Main] WSAStartup failed\n";
        return 1;
    }

    try {
        TournamentRunner runner; // путь по умолчанию
        runner.run();
    } catch (const std::exception &e) {
        std::cerr << "[Main] exception: " << e.what() << '\n';
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}
