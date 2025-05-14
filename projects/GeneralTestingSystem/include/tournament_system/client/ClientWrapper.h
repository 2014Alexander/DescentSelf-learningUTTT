// ClientWrapper.h
#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <winsock2.h>

#include "tournament_system/setup/players_programs/players_programs_data/PlayerProgramData.h"
#include "tournament_system/setup/players_programs/parameters/GeneralParams.h"
#include "tournament_system/communication/Communicator.h"
#include "tournament_system/communication/ProcessLauncher.h"
#include "tournament_system/communication/SocketListener.h"
#include "tournament_system/setup/players_programs/utils/log_utils.h"  // <-- для fatal

class ClientWrapper {
public:
    void setPlayer(std::shared_ptr<PlayerProgramData> p) {
        if (!p) {
            log_utils::fatal("[ClientWrapper] setPlayer: null PlayerProgramData");
        }
        player = std::move(p);
        rawPlayer = player.get(); // синхронизируем ссылку
        startProcessAndConnect();
    }

    void closeProcess() {
        com.SEND("EXIT");
        com.RECEIVE("OK");
    }

    void gameReset() {
        com.SEND("GAME_RESET");
        com.RECEIVE("OK");
    }

    void applyMove(uint8_t m) {
        com.SEND("APPLY_MOVE:" + std::to_string(m));
        com.RECEIVE("OK");
    }

    uint8_t makeMove() {
        com.SEND("MAKE_MOVE");
        auto ln = com.RECEIVE("MOVE");
        auto pos = ln.find(':');
        if (pos == std::string::npos) {
            log_utils::fatal("[ClientWrapper-" + getName() + "] malformed MOVE response: " + ln);
        }

        try {
            int v = std::stoi(ln.substr(pos + 1));
            if (v < 0 || v > 255) {
                log_utils::fatal("[ClientWrapper-" + getName() + "] MOVE out of range: " + std::to_string(v));
            }
            return static_cast<uint8_t>(v);
        } catch (const std::exception &e) {
            log_utils::fatal("[ClientWrapper-" + getName() + "] invalid MOVE payload: " + ln + " (" + e.what() + ")");
        }
    }


    std::string getName() const {
        return player ? player->toString() : "<none>";
    }

private:
    std::shared_ptr<PlayerProgramData> player;
    const PlayerProgramData *rawPlayer = nullptr;
    Communicator com{rawPlayer};
    SocketListener listener;

    void startProcessAndConnect() {
        const std::string cmd = player->programRunString();
        std::cout << cmd << std::endl;
        ProcessLauncher::launch(cmd);

        // слушаем входящее соединение от клиента
        SOCKET ls = listener.createAndListen(GeneralParams::refereePort);

        SOCKET sock = listener.acceptOneConnection(ls);
        closesocket(ls);

        com.descriptor = sock;
        com.RECEIVE("OK");

        if (player->getPlayerType() == PlayerType::Descent) {
            com.SEND("LOAD_MODEL:" + player->getSpecParameter());
            com.RECEIVE("OK");
        }
    }
};
