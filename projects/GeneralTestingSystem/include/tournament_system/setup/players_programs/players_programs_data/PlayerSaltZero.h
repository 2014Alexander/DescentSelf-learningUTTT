// PlayerSaltZero.h
#pragma once
#include <string>

#include "PlayerProgramData.h"
#include "../parameters/parameters_structs.h"

class PlayerSaltZero : public PlayerProgramData {
private:
    SaltZeroParams params;

public:
    explicit PlayerSaltZero(const SaltZeroParams &p, const std::string &spec = "")
        : params(p) { setSpecParameter(spec); }

    std::string programRunString() const override {
        return params.programRunString();
    }

    std::string toString() const override {
        return params.info.name;
    }

    PlayerType getPlayerType() const override {
        return params.info.type;
    }
};
