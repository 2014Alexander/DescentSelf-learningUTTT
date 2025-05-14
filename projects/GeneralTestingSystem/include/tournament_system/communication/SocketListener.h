// SocketListener.h
#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>

#include "tournament_system/setup/players_programs/utils/log_utils.h"

class SocketListener {
public:
    // Создаёт, биндит и начинает слушать на указанном порту.
    // В случае любой ошибки — вызывает log_utils::fatal и завершает программу.
    SOCKET createAndListen(int port) {
        SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock == INVALID_SOCKET) {
            log_utils::fatal(
                "[SocketListener] socket() failed, err=" +
                std::to_string(WSAGetLastError()));
        }

        sockaddr_in localAddr;
        ZeroMemory(&localAddr, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(static_cast<u_short>(port));

        if (bind(listenSock,
                 reinterpret_cast<sockaddr *>(&localAddr),
                 sizeof(localAddr)) == SOCKET_ERROR) {
            closesocket(listenSock);
            log_utils::fatal(
                "[SocketListener] bind() failed, err=" +
                std::to_string(WSAGetLastError()));
        }

        if (listen(listenSock, 1) == SOCKET_ERROR) {
            closesocket(listenSock);
            log_utils::fatal(
                "[SocketListener] listen() failed, err=" +
                std::to_string(WSAGetLastError()));
        }

        return listenSock;
    }

    // Принимает одно подключение, в случае ошибки — fatal.
    SOCKET acceptOneConnection(SOCKET listenSocket) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET acceptedSock = accept(listenSocket,
                                     reinterpret_cast<sockaddr *>(&clientAddr),
                                     &clientLen);
        if (acceptedSock == INVALID_SOCKET) {
            log_utils::fatal(
                "[SocketListener] accept() failed, err=" +
                std::to_string(WSAGetLastError()));
        }
        return acceptedSock;
    }
};
