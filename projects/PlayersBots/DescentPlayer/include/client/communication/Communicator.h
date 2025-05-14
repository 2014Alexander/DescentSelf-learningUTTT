// Communicator.h
#pragma once

#include <string>
#include <winsock2.h>
#include <iostream>

class Communicator {
public:
    SOCKET descriptor;

    Communicator()
        : descriptor(INVALID_SOCKET) {
    }

    /**
     * @brief Читает из сокета одну строку (до символа '\n').
     * @return Возвращает строку без символа '\n'. Если возникла ошибка или сокет закрыт, возвращает пустую строку "".
     */
    std::string RECEIVE_LINE() {
        char buffer[1024];
        std::string result;
        while (true) {
            int count = recv(descriptor, buffer, sizeof(buffer) - 1, 0);
            if (count <= 0) {
                // ошибка или закрытие сокета
                return ""; // пустая строка
            }
            buffer[count] = '\0';
            result += buffer;

            // Ищем символ перевода строки
            size_t pos = result.find('\n');
            if (pos != std::string::npos) {
                // извлекаем строку до \n
                std::string line = result.substr(0, pos);
                // удаляем эту часть из result
                result.erase(0, pos + 1);
                return line;
            }
            // если \n нет, продолжаем читать
        }
    }

    /**
     * @brief Отправляет строку по сокету, гарантируя перевод строки в конце.
     * @param msg Строка, которую нужно отправить.
     */
    void SEND(const std::string &msg) {
        std::string outMsg = msg;
        // Гарантируем, что в конце есть \n
        if (outMsg.find('\n') == std::string::npos) {
            outMsg += "\n";
        }
        send(descriptor, outMsg.c_str(), static_cast<int>(outMsg.size()), 0);
    }
};
