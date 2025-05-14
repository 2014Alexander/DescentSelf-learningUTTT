// string_utils.h
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace string_utils {
    // ------------------------------------------------------------
    // helpers: trim
    // ------------------------------------------------------------
    inline constexpr char whitespace_set[] = " \t\n\r\f\v";

    inline std::string_view ltrim(std::string_view sv) {
        const auto pos = sv.find_first_not_of(whitespace_set);
        return pos == std::string_view::npos ? std::string_view{} : sv.substr(pos);
    }

    inline std::string_view rtrim(std::string_view sv) {
        const auto pos = sv.find_last_not_of(whitespace_set);
        return pos == std::string_view::npos ? std::string_view{} : sv.substr(0, pos + 1);
    }

    inline std::string_view trim_view(std::string_view sv) { return rtrim(ltrim(sv)); }
    inline std::string trim(const std::string &s) { return std::string(trim_view(s)); }
    inline void trimInPlace(std::string &s) { s = trim(s); }

    // ------------------------------------------------------------
    // helpers: split
    // ------------------------------------------------------------
    inline std::pair<std::string_view, std::string_view>
    splitOnceView(std::string_view sv, char delimiter) {
        const auto pos = sv.find(delimiter);
        if (pos == std::string_view::npos) return {sv, std::string_view{}};
        return {sv.substr(0, pos), sv.substr(pos + 1)};
    }

    inline std::pair<std::string, std::string>
    splitOnce(const std::string &str, char delimiter) {
        const auto [l, r] = splitOnceView(str, delimiter);
        return {std::string(l), std::string(r)};
    }

    inline std::vector<std::string>
    split(const std::string &str, char delimiter) {
        std::vector<std::string> tokens;
        std::string current;
        current.reserve(str.size());

        for (char c: str) {
            if (c == delimiter) {
                tokens.emplace_back(std::move(current));
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        tokens.emplace_back(std::move(current));
        return tokens;
    }

    inline std::string toUpper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return s;
    }
} // namespace string_utils
