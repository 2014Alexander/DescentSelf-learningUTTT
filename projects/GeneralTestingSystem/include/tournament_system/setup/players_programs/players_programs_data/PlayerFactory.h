// PlayerFactory.h
#pragma once

#include <memory>
#include <string>
#include "PlayerProgramData.h"
#include "PlayerSaltZero.h"
#include "PlayerDescent.h"
#include "PlayerMinimax.h"
#include "../parameters/ParamsManager.h"
#include "../players_pairs/pairs_configs/pair_config_structs/PlayerConfig.h" // Добавлен для использования PlayerConfig

// Фабрика для создания объектов-игроков
class PlayerFactory {
public:
    // Конструктор принимает ссылку на ParamsManager, из которого будут доставаться параметры
    explicit PlayerFactory(const ParamsManager &paramsManager)
        : paramsManager(paramsManager) {
    }

    // Метод, который принимает имя игрока и опциональный специальный параметр.
    // В зависимости от того, где найден playerName в картах параметров (Descent, Minimax, SaltZero),
    // создается соответствующий объект.
    // Возвращает уникальный указатель на базовый класс PlayerProgramData, либо nullptr, если игрок с таким именем не найден.
    std::unique_ptr<PlayerProgramData> createPlayer(const std::string &playerName,
                                                    const std::string &specParameter = "") const {
        // Поиск среди Descent-параметров
        auto descentIt = paramsManager.getDescentParamsMap().find(playerName);
        if (descentIt != paramsManager.getDescentParamsMap().end()) {
            return std::make_unique<PlayerDescent>(descentIt->second, specParameter);
        }

        // Поиск среди Minimax-параметров
        auto minimaxIt = paramsManager.getMinimaxParamsMap().find(playerName);
        if (minimaxIt != paramsManager.getMinimaxParamsMap().end()) {
            return std::make_unique<PlayerMinimax>(minimaxIt->second, specParameter);
        }

        // Поиск среди SaltZero-параметров
        auto saltZeroIt = paramsManager.getSaltZeroParamsMap().find(playerName);
        if (saltZeroIt != paramsManager.getSaltZeroParamsMap().end()) {
            return std::make_unique<PlayerSaltZero>(saltZeroIt->second, specParameter);
        }

        // Если не найдены параметры для игрока с данным именем – возвращаем nullptr или можно выбросить исключение
        return nullptr;
    }

    // Метод, принимающий объект PlayerConfig для создания игрока
    std::unique_ptr<PlayerProgramData> createPlayer(const PlayerConfig &config) const {
        return createPlayer(config.playerName, config.specParameter);
    }

private:
    // Ссылка на менеджер параметров, содержащий карты с параметрами для разных типов игроков.
    const ParamsManager &paramsManager;
};
