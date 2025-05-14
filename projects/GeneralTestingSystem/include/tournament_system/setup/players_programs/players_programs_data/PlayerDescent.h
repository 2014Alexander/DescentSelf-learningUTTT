// PlayerDescent.h
#pragma once
#include <string>

#include "PlayerProgramData.h"
#include "../parameters/parameters_structs.h"

class PlayerDescent : public PlayerProgramData {
private:
    DescentParams params;

public:
    explicit PlayerDescent(const DescentParams &p, const std::string &spec = "")
        : params(p) { setSpecParameter(spec); }

    std::string programRunString() const override { return params.programRunString(); }

    std::string getSpecParameter() const override {
        return params.checkpointsPath + "model_checkpoint.weights_" + specParameter + ".h5";
    }

    std::string toString() const override { return params.info.name + "-" + specParameter; }
    PlayerType getPlayerType() const override { return params.info.type; }
};
