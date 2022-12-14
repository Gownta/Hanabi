#pragma once

#include "common/logging/logging.h"
#include "experimental/njormrod/hanabi/GameState.h"

class ModularEngine {
 public:
  ModularEngine(View v, bool preferLowerValues, bool preferOlderCards)
      : view_(v),
        preferLowerValues_(preferLowerValues),
        preferOlderCards_(preferOlderCards) {}

  // Returns
  //   {checksum mod 5,
  //    [positions of playable cards for next n players]}
  std::pair<int, small_vector<int, 3>> computeModular(int n);

  void update(const Action& action);

  // Returns positions to play for current player
  // and all remaining players
  // also resets checksum to -1, since we've acted upon it
  std::optional<small_vector<int, 3>> toPlay();

 private:
  View view_;
  bool preferLowerValues_;
  bool preferOlderCards_;

  int checksum_ = -1;
  small_vector<int, 3> afters_; // what players after you should do
};

// Given an existing modulo play pattern, and your desired positions,
// how many more players have something to do
int countExtraPlays(
    const std::optional<small_vector<int, 3>>& youAndAfters,
    const small_vector<int, 3>& hintPositions);
