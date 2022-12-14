#pragma once

#include "Action.h"
#include "GameState.h"
#include "Knowledge.h"

class HPlayer {
 public:
  HPlayer(GameView v);

  Decision act();
  void inform(const Action& action); // shifted so self is 0

 private:
  GameView v;
  // std::array<Knowledge, 5> pub; // 0 is self
  // Knowledge priv;
};
