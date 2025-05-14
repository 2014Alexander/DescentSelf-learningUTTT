// ParamsManager.h
#pragma once

#include <string>
#include <unordered_map>

#include "parameters_structs.h"

class ParamsManager {
public:
    explicit ParamsManager(std::string configPath);

    const std::string &getConfigPath() const { return paramsFilePath; }
    const std::unordered_map<std::string, DescentParams> &getDescentParamsMap() const { return descentParamsMap; }
    const std::unordered_map<std::string, MinimaxParams> &getMinimaxParamsMap() const { return minimaxParamsMap; }
    const std::unordered_map<std::string, SaltZeroParams> &getSaltZeroParamsMap() const { return saltZeroParamsMap; }

private:
    std::string paramsFilePath;
    std::unordered_map<std::string, DescentParams> descentParamsMap;
    std::unordered_map<std::string, MinimaxParams> minimaxParamsMap;
    std::unordered_map<std::string, SaltZeroParams> saltZeroParamsMap;

    void loadParams();
};
