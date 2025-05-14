// ResultsLogger.cpp
#include "tournament_system/referee/ResultsLogger.h"
#include "tournament_system/tournament/Referee.h"
#include "tournament_system/setup/players_programs/utils/log_utils.h"

ResultsLogger::ResultsLogger(const std::string &filePath, Referee &r)
    : resultFile(filePath, std::ios::app)
      , ref(r) {
    if (!resultFile.is_open())
        log_utils::fatal("[ResultsLogger] cannot open " + filePath);
}

void ResultsLogger::addResult(int score) {
    resultFile << ref.clientX->getName() << " - "
            << ref.clientO->getName() << " : "
            << score << '\n';
    resultFile.flush();
}
