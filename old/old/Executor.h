#pragma once

#include <functional>
#include <memory>
#include "GameState.h"
#include "Player.h"

// returns the score
int execute(
    int numPlayers,
    std::function<std::unique_ptr<Player>(const GameState&, int)> playerBuilder,
    unsigned seed,
    bool interactive);
