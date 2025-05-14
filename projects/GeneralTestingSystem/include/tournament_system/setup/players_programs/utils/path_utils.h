// path_utils.h
#pragma once
#include <string>
#include <algorithm>

namespace path_utils {
    /// Заменяет все '\' на '/' в любом пути.
    inline std::string normalize_slashes(std::string p) {
        std::replace(p.begin(), p.end(), '\\', '/');
        return p;
    }

    /// Гарантирует, что путь кончается '/', если это директория.
    inline std::string ensure_trailing_slash(std::string p) {
        if (!p.empty() && p.back() != '/')
            p.push_back('/');
        return p;
    }

    /// Оборачивает строку в двойные кавычки,
    /// экранируя всё, что надо внутри.
    inline std::string quote(const std::string &arg) {
        std::string out;
        out.reserve(arg.size() + 2);
        out.push_back('"');
        for (char c: arg) {
            if (c == '"') out += "\\\"";
            else out.push_back(c);
        }
        out.push_back('"');
        return out;
    }
}
