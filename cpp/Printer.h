#pragma once

#include <iosfwd>
#include "Action.h"
#include "GameState.h"

std::ostream& operator<<(std::ostream& os, Card c);

void printGame(
    const NaturesGameState& g,
    const Action* b = nullptr,
    const Action* e = nullptr);
