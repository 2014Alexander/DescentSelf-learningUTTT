// PairsConfigReader.cpp
#include "tournament_system/setup/players_programs/players_pairs/pairs_configs/PairsConfigReader.h"
#include <fstream>

#include "tournament_system/setup/players_programs/utils/log_utils.h"
#include "tournament_system/setup/players_programs/utils/string_utils.h"
#include "tournament_system/setup/players_programs/utils/parser_utils.h"

using namespace string_utils;
using namespace parser_utils;
using log_utils::error;
using log_utils::warning;

PairsConfigReader::PairsConfigReader(std::string configPath)
    : configPath_(std::move(configPath)) {
    readPlayersPairsFromConfigFile();
}

const std::vector<PlayersPairConfigData> &
PairsConfigReader::getGameConfigPairs() const {
    return gameConfigPairs;
}

void PairsConfigReader::readPlayersPairsFromConfigFile() {
    std::ifstream fin(configPath_);
    if (!fin.is_open()) {
        error("Не удалось открыть файл: " + configPath_);
        return;
    }

    std::string line;
    // Ищем секцию --- GAMES ---
    while (std::getline(fin, line) && trim_view(line) != "--- GAMES ---") {
    }
    if (fin.eof()) {
        error("Секция \"--- GAMES ---\" не найдена");
        return;
    }

    static constexpr std::string_view SEP = " - ";
    while (std::getline(fin, line)) {
        auto trimmed = trim_view(line);
        if (isCommentOrEmpty(trimmed)) continue;

        auto sepPos = trimmed.find(SEP);
        if (sepPos == std::string::npos) {
            warning("Неправильный формат строки игр: \"" + std::string(trimmed) + "\"");
            continue;
        }

        std::string_view left = trimmed.substr(0, sepPos);
        std::string_view right = trimmed.substr(sepPos + SEP.size());
        left = trim_view(left);
        right = trim_view(right);

        PlayersPairConfigData pair;
        pair.first = parsePlayerConfig(left);
        pair.second = parsePlayerConfig(right);
        gameConfigPairs.push_back(std::move(pair));
    }
}
