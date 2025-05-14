// ClientMain.h
#pragma once

#include <string>
#include <winsock2.h>

#include "communication/Communicator.h"
#include "communication/TcpConnector.h"
#include "selfplay/Player.h"

/**
 * @brief Класс основной логики клиента, который подключается к рефери-серверу
 *        и обрабатывает команды от него (LOAD_MODEL, GAME_RESET, APPLY_MOVE, MAKE_MOVE, EXIT).
 */
class ClientMain {
public:
    Communicator communicator; ///< Для отправки/приёма строк по TCP
    SOCKET socketDescriptor; ///< Основной сокет
    int port;
    float timePerMove;
    Player *player; ///< Указатель на текущего игрока (Player)


public:
    /**
     * @param port Порт, на который будем подключаться (сервер-рефери).
     * @param timePerMove Лимит времени на ход.
     */
    ClientMain(int port, float timePerMove)
        : socketDescriptor(INVALID_SOCKET)
          , port(port)
          , timePerMove(timePerMove)
          , player(nullptr) {
        // communicator.descriptor = INVALID_SOCKET по умолчанию
    }

    /**
     * @brief Основной цикл работы клиента.
     *  1) Подключаемся к серверу (doConnect())
     *  2) Отправляем "OK" (сигнал готовности)
     *  3) Считываем команды в цикле, пока сервер не закроет соединение
     */
    void mainLoop() {
        // Подключение к серверу (содержит и WSAStartup)
        if (!doConnect()) {
            return; // Если не получилось подключиться, выходим
        }

        // Сообщаем рефери, что клиент готов
        communicator.SEND("OK");

        // Основной цикл приёма команд
        while (true) {
            std::string line = communicator.RECEIVE_LINE();
            if (line.empty()) {
                // Сокет закрыт или ошибка
                std::cerr << "[ClientMain] Server closed connection or error.\n";
                break;
            }

            // Парсим строку вида "COMMAND[:params]"
            auto [command, params] = parseLine(line);

            if (command == "EXIT") {
                // Ответим "OK" и закроем соединение
                communicator.SEND("OK");
                closesocket(socketDescriptor);
                break;
            }
            if (command == "GAME_RESET") {
                resetGame();
                communicator.SEND("OK");
            } else if (command == "APPLY_MOVE") {
                applyMove(params);
                communicator.SEND("OK");
            } else if (command == "MAKE_MOVE") {
                makeMove();
            } else {
                std::cerr << "[ClientMain] Unknown command: " << command << "\n";
                // Можно игнорировать или отправлять что-то вроде communicator.SEND("ERR");
            }
        }

        // Выходим из цикла -> завершение
        if (player != nullptr) {
            delete player;
            player = nullptr;
        }
        // WinSock уже очищаем при ошибке подключения или когда захотим
        WSACleanup();
    }

private:
    /**
     * @brief Выполняет WSAStartup и подключение к серверу,
     *        сохраняя дескриптор сокета в socketDescriptor.
     * @return true, если удалось подключиться; false — при ошибке.
     */
    bool doConnect() {
        // Инициализация WinSock
        WSADATA wsaData;
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaErr != 0) {
            std::cerr << "[ClientMain] WSAStartup failed: " << wsaErr << "\n";
            return false;
        }

        // Подключаемся к локальному серверу (port)
        socketDescriptor = TcpConnector::connectToServer(port);
        if (socketDescriptor == INVALID_SOCKET) {
            std::cerr << "[ClientMain] Failed to connect to server on port=" << port << "\n";
            WSACleanup();
            return false;
        }
        communicator.descriptor = socketDescriptor;
        return true;
    }

    /**
     * @brief Разбивает строку вида "COMMAND[:PARAMS]" на (command, params).
     * @param line Исходная строка.
     * @return Пара (command, params).
     */
    static std::pair<std::string, std::string> parseLine(const std::string &line) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            // Нет двоеточия => вся строка это команда
            return {line, ""};
        } else {
            std::string cmd = line.substr(0, colonPos);
            std::string prms = line.substr(colonPos + 1);
            return {cmd, prms};
        }
    }


    /**
     * @brief Сброс игрового состояния:
     *        удаляем старый Player и создаём новый.
     */
    void resetGame() {
        if (player != nullptr) {
            delete player;
            player = nullptr;
        }
        player = new Player(timePerMove);
    }

    /**
     * @brief Применяет ход, если player есть.
     * @param param — строка с числовым значением (moveByte).
     */
    void applyMove(const std::string &param) {
        if (!player) return;
        int moveByte = std::stoi(param);
        player->applyLocalMove(static_cast<uint8_t>(moveByte));
    }

    /**
     * @brief Запрос на ход от рефери.
     *        Вызывает player->makeNextMove() и отправляет "MOVE:xx".
     */
    void makeMove() {
        if (!player) {
            std::cerr << "[ClientMain] No player for MAKE_MOVE!\n";
            return;
        }
        uint8_t moveByte = player->makeNextMove();
        std::string answer = "MOVE:" + std::to_string(moveByte);
        communicator.SEND(answer);
    }
};
