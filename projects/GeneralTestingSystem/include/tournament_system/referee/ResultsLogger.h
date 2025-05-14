// ResultsLogger.h
#pragma once

#include <fstream>
#include <string>

class Referee;

/// Добавляет строку вида  «Игрок1 - Игрок2 : score»  в results‑файл
class ResultsLogger {
public:
    ResultsLogger(const std::string &filePath, Referee &r);

    void addResult(int score);

private:
    std::ofstream resultFile;
    Referee &ref;
};
