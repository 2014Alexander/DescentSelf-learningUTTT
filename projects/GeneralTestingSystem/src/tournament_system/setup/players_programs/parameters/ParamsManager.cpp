// ParamsManager.cpp
#include "tournament_system/setup/players_programs/parameters/ParamsManager.h"

#include <fstream>
#include <sstream>

#include "tournament_system/setup/players_programs/utils/log_utils.h"
#include "tournament_system/setup/players_programs/utils/string_utils.h"
#include "tournament_system/setup/players_programs/utils/parser_utils.h"

using namespace string_utils;
using namespace parser_utils;
using log_utils::error;
using log_utils::debug;

ParamsManager::ParamsManager(std::string configPath)
    : paramsFilePath(std::move(configPath)) {
    loadParams();
}

void ParamsManager::loadParams() {
    std::ifstream fin(paramsFilePath);
    if (!fin.is_open()) {
        error("Не удалось открыть файл параметров: " + paramsFilePath);
        return;
    }

    bool inParameters = false, inPlayers = false;
    std::string line;

    while (std::getline(fin, line)) {
        auto t = trim_view(line);

        if (t == "--- PARAMETERS ---") {
            inParameters = true;
            inPlayers = false;
            continue;
        }
        if (t == "--- PLAYER_PROGRAMS ---") {
            inParameters = false;
            inPlayers = true;
            continue;
        }
        if (t == "--- GAMES ---") break;

        /* ---------- global parameters ---------- */
        if (inParameters && !isCommentOrEmpty(t)) {
            auto [k,v] = parseKeyValue(t);
            if (k == "REFEREE_PORT") GeneralParams::refereePort = std::stoi(v);
            else if (k == "PAIR_GAMES_NUM") GeneralParams::pairGamesNum = std::stoi(v);
            else if (k == "RESULTS_FILE_PATH") GeneralParams::resultsFilePath = v;
            continue;
        }

        /* ---------- player definitions ---------- */
        if (!inPlayers || t.empty() || t.find('{') == std::string_view::npos)
            continue;

        auto [head, _] = splitOnceView(t, '{');
        auto [type_sv, name_sv] = splitOnceView(head, ':');
        PlayerType ptype = parsePlayerType(type_sv);
        std::string name = std::string(trim_view(name_sv));

        std::stringstream blk;
        for (auto &l: readBlock(fin)) blk << l << '\n';

        std::string k, v, ln;
        switch (ptype) {
            case PlayerType::Descent: {
                DescentParams dp;
                dp.info.name = name;
                dp.info.type = ptype;
                while (std::getline(blk, ln)) {
                    std::tie(k, v) = parseKeyValue(trim_view(ln));
                    if (k == "TIME_PER_MOVE") dp.timePerMove = std::stof(v);
                    else if (k == "DESCENT_PLAYER") dp.descentPlayerPath = v;
                    else if (k == "PYTHON_SCRIPTS") dp.pythonScriptsPath = v;
                    else if (k == "CHECKPOINTS_PATH") dp.checkpointsPath = v;
                }
                descentParamsMap.emplace(name, std::move(dp));
            }
            break;

            case PlayerType::Minimax: {
                MinimaxParams mp;
                mp.info.name = name;
                mp.info.type = ptype;
                while (std::getline(blk, ln)) {
                    std::tie(k, v) = parseKeyValue(trim_view(ln));
                    if (k == "MINIMAX_PLAYER") mp.minimaxPlayerPath = v;
                    else if (k == "TIME_PER_MOVE") mp.timePerMove = std::stof(v);
                }
                minimaxParamsMap.emplace(name, std::move(mp));
            }
            break;

            case PlayerType::SaltZero: {
                SaltZeroParams sz;
                sz.info.name = name;
                sz.info.type = ptype;
                while (std::getline(blk, ln)) {
                    std::tie(k, v) = parseKeyValue(trim_view(ln));
                    if (k == "PYTHON_EXE") sz.pythonExePath = v;
                    else if (k == "PYTHON_SCRIPT") sz.pythonScriptPath = v;
                    else if (k == "TIME_PER_MOVE") sz.timePerMove = std::stof(v);
                }
                saltZeroParamsMap.emplace(name, std::move(sz));
            }
            break;

            default:
                error("Неизвестный тип игрока: " + std::string(trim_view(type_sv)));
        }
    }
    debug("ParamsManager: параметры загружены");
}
