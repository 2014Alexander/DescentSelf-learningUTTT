// PlayerConfig.h
#pragma once
#include <string>

// Структура для хранения конфигурационных данных об игроке
struct PlayerConfig {
    std::string playerName; // Имя, например, "Descent1", "SaltZero", "Minimax" и т.д.
    std::string specParameter; // Специальный параметр, например, "check001", "0925", "4" и т.д.
};
