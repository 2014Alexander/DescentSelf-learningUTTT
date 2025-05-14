// GeneralParams.h
#pragma once
#include <string>

// Глобальные параметры (отличные от состояния игрока)
class GeneralParams {
public:
    inline static std::string resultsFilePath;
    inline static int refereePort = 0;
    inline static int pairGamesNum = 0;
};
