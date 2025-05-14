// PairsConfigReader.h
#pragma once

#include <vector>
#include "pair_config_structs/PlayersPairConfigData.h"

class PairsConfigReader {
public:
    explicit PairsConfigReader(std::string configPath);

    const std::vector<PlayersPairConfigData> &getGameConfigPairs() const;

private:
    void readPlayersPairsFromConfigFile();

    std::string configPath_;
    std::vector<PlayersPairConfigData> gameConfigPairs;
};
