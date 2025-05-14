// PlayersPairsProvider.h
#pragma once

#include <vector>
#include <memory>
#include <string>

#include "tournament_system/setup/players_programs/parameters/ParamsManager.h"
#include "tournament_system/setup/players_programs/players_programs_data/PlayerProgramData.h"
#include "tournament_system/setup/players_programs/players_programs_data/PlayerFactory.h"
#include "tournament_system/setup/players_programs/players_pairs/pairs_configs/PairsConfigReader.h"

class PlayersPairsProvider {
public:
    // Явный тип пары, теперь с toString()
    struct PlayerPair {
        std::shared_ptr<PlayerProgramData> first;
        std::shared_ptr<PlayerProgramData> second;

        // Просто пример: "Descent1-check001 - SaltZero2"
        std::string toString() const {
            return first->toString()
                   + " - "
                   + second->toString();
        }
    };

    // Конструктор принимает готовый ParamsManager
    explicit PlayersPairsProvider(const ParamsManager &paramsManager);

    // Метод возвращает вектор пар игроков
    std::vector<PlayerPair> getPlayerPairs() const;

private:
    PlayerFactory factory; // для создания конкретных игроков
    PairsConfigReader configReader; // для чтения конфигурации пар
};
