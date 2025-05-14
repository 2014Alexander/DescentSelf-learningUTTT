// parameters_structs.h
#pragma once

#include <string>
#include "GeneralParams.h"
#include "tournament_system/setup/players_programs/utils/player_type.h"
#include "tournament_system/setup/players_programs/utils/path_utils.h"

struct PlayerInfo {
    std::string name;
    PlayerType type = PlayerType::Unknown;
};

struct DescentParams {
    PlayerInfo info;
    float timePerMove;
    std::string descentPlayerPath; // путь к .exe
    std::string pythonScriptsPath; // путь к папке со скриптами
    std::string checkpointsPath;

    std::string programRunString() const {
        using namespace path_utils;
        // 1) exe — просто нормализуем слеши, ДЛЯ ФАЙЛА НЕ добавляем '/'
        auto exe = normalize_slashes(descentPlayerPath);
        // 2) скрипты — нормализуем и добавляем '/'
        auto scripts = ensure_trailing_slash(normalize_slashes(pythonScriptsPath));

        return quote(exe)
               + " " + std::to_string(GeneralParams::refereePort)
               + " " + quote(scripts)
               + " " + std::to_string(timePerMove);
    }
};

struct MinimaxParams {
    PlayerInfo info;
    std::string minimaxPlayerPath;
    float timePerMove;

    std::string programRunString() const {
        using namespace path_utils;
        auto exe = normalize_slashes(minimaxPlayerPath);
        return quote(exe)
               + " " + std::to_string(GeneralParams::refereePort)
               + " " + std::to_string(timePerMove);
    }
};

struct SaltZeroParams {
    PlayerInfo info;
    std::string pythonExePath;
    std::string pythonScriptPath; // тоже папка
    float timePerMove;

    std::string programRunString() const {
        using namespace path_utils;
        auto exe = normalize_slashes(pythonExePath);
        auto script = normalize_slashes(pythonScriptPath);

        return quote(exe)
               + " " + quote(script)
               + " " + std::to_string(GeneralParams::refereePort)
               + " " + std::to_string(timePerMove);
    }
};
