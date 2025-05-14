// ProcessLauncher.h
#pragma once

#include <string>
#include <windows.h>
#include "tournament_system/setup/players_programs/utils/log_utils.h"

/// Запускает внешний процесс по уже сформированной командной строке
/// (см. PlayerProgramData::programRunString()).
/// В случае ошибки – вызывает log_utils::fatal и завершает программу.
class ProcessLauncher {
public:
    static void launch(const std::string &commandLine) {
        STARTUPINFOA si{sizeof(si)};
        PROCESS_INFORMATION pi{};

        // CreateProcessA требует LPSTR – используем &cmd[0]
        std::string cmd = commandLine;
        BOOL ok = CreateProcessA(
            nullptr, // lpApplicationName
            cmd.empty() ? nullptr : &cmd[0], // lpCommandLine
            nullptr, nullptr, // атрибуты безопасности
            FALSE, // дескрипторы не наследуют
            0, // флаги создания
            nullptr, nullptr, // окружение / рабочий каталог
            &si, &pi
        );
        if (!ok) {
            log_utils::fatal(
                "[ProcessLauncher] CreateProcessA failed, err=" +
                std::to_string(GetLastError())
            );
        }

        // Закрываем хендлы процесса и потока — нам они не нужны
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
};
