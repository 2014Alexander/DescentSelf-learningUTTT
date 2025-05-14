// parser_utils.h
#pragma once
#include <istream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>

#include "player_type.h"
#include "string_utils.h"
#include "../players_pairs/pairs_configs/pair_config_structs/PlayerConfig.h"

namespace parser_utils {
    // Таблица соответствий строк → enum
    static const std::unordered_map<std::string, PlayerType> kPlayerTypeMap = {
        {"DESCENT", PlayerType::Descent},
        {"MINIMAX", PlayerType::Minimax},
        {"SALT_ZERO", PlayerType::SaltZero}
    };

    /// Парсит строку вида "DESCENT"/"Minimax" → соответствующий PlayerType.
    inline PlayerType parsePlayerType(std::string_view sv) {
        using namespace string_utils;
        // Тримим, переводим в std::string и в верхний регистр
        std::string key = toUpper(std::string(trim_view(sv)));
        auto it = kPlayerTypeMap.find(key);
        return it == kPlayerTypeMap.end()
                   ? PlayerType::Unknown
                   : it->second;
    }

    // ------------------------------------------------------------
    // Вспомогательные проверки
    // ------------------------------------------------------------
    inline bool isCommentOrEmpty(std::string_view sv) {
        sv = string_utils::trim_view(sv);
        return sv.empty() || sv.starts_with('#') || sv.starts_with("//");
    }

    // ------------------------------------------------------------
    // "KEY: VALUE"  -> {key, value}  (trim‑нутые)
    // ------------------------------------------------------------
    inline std::pair<std::string, std::string>
    parseKeyValue(std::string_view line) {
        auto [k, v] = string_utils::splitOnceView(line, ':');
        return {
            string_utils::trim(std::string(k)),
            string_utils::trim(std::string(v))
        };
    }

    // ------------------------------------------------------------
    // Считать блок строк до первой '}', пропуская пустые/комментарии
    // ------------------------------------------------------------
    inline std::vector<std::string>
    readBlock(std::istream &fin) {
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(fin, line)) {
            auto trimmed = string_utils::trim_view(line);
            if (trimmed.find('}') != std::string_view::npos) break;
            if (isCommentOrEmpty(trimmed)) continue;
            lines.emplace_back(line);
        }
        return lines;
    }

    // ------------------------------------------------------------
    // "PlayerName:spec" -> PlayerConfig
    // ------------------------------------------------------------
    inline PlayerConfig
    parsePlayerConfig(std::string_view part) {
        PlayerConfig cfg;
        auto [name, spec] = string_utils::splitOnceView(part, ':');
        cfg.playerName = string_utils::trim(std::string(name));
        cfg.specParameter = string_utils::trim(std::string(spec));
        return cfg;
    }
} // namespace parser_utils
