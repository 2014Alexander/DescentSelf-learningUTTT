// player_type.h
#pragma once
#include <ostream>

enum class PlayerType {
    Descent,
    Minimax,
    SaltZero,
    Unknown
};

inline const char *toString(PlayerType t) {
    switch (t) {
        case PlayerType::Descent: return "Descent";
        case PlayerType::Minimax: return "Minimax";
        case PlayerType::SaltZero: return "SaltZero";
        default: return "Unknown";
    }
}

// для прямого вывода в поток
inline std::ostream &operator<<(std::ostream &os, PlayerType t) {
    return os << toString(t);
}
