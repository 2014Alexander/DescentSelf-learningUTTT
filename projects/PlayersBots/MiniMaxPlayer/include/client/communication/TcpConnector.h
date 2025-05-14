// TcpConnector.h
#pragma once

#include <winsock2.h>
#include <windows.h>
#include <iostream>

class TcpConnector {
public:
    /**
     * @brief Подключается к локальному серверу по указанному порту.
     * @param port Порт, на котором слушает сервер (рефери).
     * @return Успешно подключённый сокет или INVALID_SOCKET при ошибке.
     */
    static SOCKET connectToServer(int port) {
        // 1) Создаём TCP-сокет
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "[TcpConnector] socket() failed, err=" << WSAGetLastError() << std::endl;
            return INVALID_SOCKET;
        }

        // 2) Формируем адрес (127.0.0.1) и порт
        sockaddr_in serverAddr;
        ZeroMemory(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(static_cast<u_short>(port));
        // INADDR_LOOPBACK — это 127.0.0.1 в сетевом порядке
        serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        // 3) Пытаемся соединиться
        int ret = ::connect(sock, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr));
        if (ret == SOCKET_ERROR) {
            std::cerr << "[TcpConnector] connect() failed, err=" << WSAGetLastError() << std::endl;
            closesocket(sock);
            return INVALID_SOCKET;
        }

        // Успешное подключение
        return sock;
    }
};
