#include "HPlayer.h"

HPlayer::HPlayer(GameView _v) : v(_v) {}

Decision HPlayer::act() {
  return PlayDecision{0};
}

void HPlayer::inform(const Action& action) {}

// GameView v;
// std::array<Knowledge, 5> pub; // 0 is self
// Knowledge priv;
