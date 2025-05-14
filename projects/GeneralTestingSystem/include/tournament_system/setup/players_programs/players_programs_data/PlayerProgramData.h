// PlayerProgramData.h
#pragma once
#include <string>
#include "tournament_system/setup/players_programs/utils/player_type.h"

/// Абстрактный базовый класс для данных об игроке
class PlayerProgramData {
protected:
    std::string specParameter; // дополнительный параметр (checkpoint, версия …)
public:
    virtual std::string programRunString() const = 0;

    virtual std::string toString() const = 0;

    virtual PlayerType getPlayerType() const = 0;

    virtual void setSpecParameter(const std::string &p) { specParameter = p; }
    virtual std::string getSpecParameter() const { return specParameter; }

    virtual ~PlayerProgramData() = default;
};
