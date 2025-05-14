// PlayersPairsProvider.cpp
#include "tournament_system/setup/players_programs/players_pairs/PlayersPairsProvider.h"

#include "tournament_system/setup/players_programs/utils/log_utils.h"

using log_utils::warning;

PlayersPairsProvider::PlayersPairsProvider(const ParamsManager &pm)
    : factory(pm), configReader(pm.getConfigPath()) {
}

std::vector<PlayersPairsProvider::PlayerPair>
PlayersPairsProvider::getPlayerPairs() const {
    std::vector<PlayerPair> result;

    for (const auto &cfg: configReader.getGameConfigPairs()) {
        auto p1 = factory.createPlayer(cfg.first);
        auto p2 = factory.createPlayer(cfg.second);

        if (!p1 || !p2) {
            warning("Не удалось создать игрока: " +
                    std::string(!p1 ? cfg.first.playerName : cfg.second.playerName));
            continue;
        }
        result.push_back({std::move(p1), std::move(p2)});
    }
    return result;
}
