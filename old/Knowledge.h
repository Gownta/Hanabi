#pragma once

#include "Action.h"
#include "GameState.h"

class Knowledge {
 public:
  Knowledge() = default;

  void inform(const Action& action);
};
