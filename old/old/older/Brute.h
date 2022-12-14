#pragma once

#include <vector>
#include "GameState.h"

class Brute {
 public:
  Brute(int pos, Token omni);
  std::vector<Decision> actions(const GameState& gs) const;

 private:
  int pos;
  Token tok;
};
