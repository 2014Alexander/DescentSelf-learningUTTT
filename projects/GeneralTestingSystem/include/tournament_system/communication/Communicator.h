// Communicator.h

#pragma once

#include <string>
#include <winsock2.h>
#include <iostream>
#include "tournament_system/setup/players_programs/players_programs_data/PlayerProgramData.h"
#include "tournament_system/setup/players_programs/utils/log_utils.h"  // <-- добавили

/// Обмен текстовыми сообщениями “COMMAND[:params]\n”
class Communicator {
    const PlayerProgramData *const &player; // ссылка на указатель – обновляется снаружи
public:
    SOCKET descriptor = INVALID_SOCKET;

    explicit Communicator(const PlayerProgramData *const &pl)
        : player(pl) {
    }

    std::string RECEIVE(const std::string &expected) {
        char buf[1024];
        std::string acc;
        while (true) {
            int n = recv(descriptor, buf, sizeof(buf) - 1, 0);
            if (n <= 0) {
                // socket closed or error
                log_utils::fatal("[Communicator-" + player->toString() + "] socket closed");
            }
            buf[n] = '\0';
            acc += buf;
            auto nl = acc.find('\n');
            if (nl == std::string::npos) continue;

            std::string line = acc.substr(0, nl);
            acc.erase(0, nl + 1);

            std::string cmd = line.substr(0, line.find(':'));
            if (cmd != expected) {
                // получили неожиданный ответ — считаем это фатальной ошибкой
                log_utils::fatal("[Communicator-" + player->toString() +
                                 "] got '" + cmd + "' but expected '" + expected + "'");
            }
            std::cout << "[Referee] <= [" << player->toString() << "]: "
                    << line << '\n';
            return line;
        }
    }

    void SEND(std::string msg) {
        if (msg.empty() || msg.back() != '\n') msg.push_back('\n');
        int sent = send(descriptor, msg.c_str(), static_cast<int>(msg.size()), 0);
        if (sent == SOCKET_ERROR) {
            log_utils::fatal("[Communicator-" + player->toString() +
                             "] send() failed, err=" + std::to_string(WSAGetLastError()));
        }
        std::cout << "[Referee] => [" << player->toString() << "]: "
                << msg.substr(0, msg.size() - 1) << '\n';
    }
};
