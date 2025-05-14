// TournamentRunner.h
#pragma once
#include <string>
#include "tournament_system/setup/players_programs/parameters/ParamsManager.h"
#include "tournament_system/setup/players_programs/players_pairs/PlayersPairsProvider.h"
#include "tournament_system/tournament/Referee.h"

class TournamentRunner {
public:
    explicit TournamentRunner(std::string cfgPath);

    TournamentRunner();

    ~TournamentRunner();

    void run();

private:
    ParamsManager pm;
    PlayersPairsProvider provider;
};
