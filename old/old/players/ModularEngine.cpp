#include "ModularEngine.h"
#include <algorithm>

bool isPlayable(
    const Card& card,
    const std::array<Value, NUM_COLOURS>& played,
    bool taken[5]) {
  return played[card.colour] + 1 == card.value && !taken[card.colour];
}

int modularIndexNewestLowest(
    const Hand& hand,
    const std::array<Value, NUM_COLOURS>& played,
    bool taken[5]) {
  int valToPlay = 6;
  int posToPlay = 4;
  for (int i = 0; i < hand.size(); ++i) {
    const auto& card = hand[i];
    if (isPlayable(card, played, taken) && card.value <= valToPlay) {
      valToPlay = card.value;
      posToPlay = i;
    }
  }
  return posToPlay;
}

int modularIndexOldestLowest(
    const Hand& hand,
    const std::array<Value, NUM_COLOURS>& played,
    bool taken[5]) {
  int valToPlay = 6;
  int posToPlay = 4;
  for (int i = hand.size() - 1; i >= 0; --i) {
    const auto& card = hand[i];
    if (isPlayable(card, played, taken) && card.value <= valToPlay) {
      valToPlay = card.value;
      posToPlay = i;
    }
  }
  return posToPlay;
}

int modularIndexNewest(
    const Hand& hand,
    const std::array<Value, NUM_COLOURS>& played,
    bool taken[5]) {
  for (int i = hand.size() - 1; i >= 0; --i) {
    if (isPlayable(hand[i], played, taken)) {
      return i;
    }
  }
  return 4;
}

int modularIndexOldest(
    const Hand& hand,
    const std::array<Value, NUM_COLOURS>& played,
    bool taken[5]) {
  for (int i = 0; i < hand.size(); ++i) {
    if (isPlayable(hand[i], played, taken)) {
      return i;
    }
  }
  return 4;
}

int modularIndex(
    const Hand& hand,
    const std::array<Value, NUM_COLOURS>& played,
    bool preferLowerValues,
    bool preferOlderCards,
    bool taken[5]) {
  int posToPlay;
  if (preferLowerValues) {
    if (preferOlderCards) {
      posToPlay = modularIndexOldestLowest(hand, played, taken);
    } else {
      posToPlay = modularIndexNewestLowest(hand, played, taken);
    }
  } else {
    if (preferOlderCards) {
      posToPlay = modularIndexOldest(hand, played, taken);
    } else {
      posToPlay = modularIndexNewest(hand, played, taken);
    }
  }
  if (posToPlay != 4) {
    taken[hand[posToPlay].colour] = true;
  }
  return posToPlay;
}

std::pair<int, small_vector<int, 3>> ModularEngine::computeModular(int n) {
  int cksm = view_.getScore();
  small_vector<int, 3> playables;

  const auto& played = view_.getPlayed();
  bool taken[5] = {false}; // has colour i been allocated?
  for (int i = n; i >= 1; --i) {
    int who = (view_.getMyPos() + i) % 4;
    int playable = modularIndex(
        view_.getHand(who),
        played,
        preferLowerValues_,
        preferOlderCards_,
        taken);
    playables.push_back(playable);
    cksm += playable;
  }
  std::reverse(playables.begin(), playables.end());

  return {cksm % 5, playables};
}

void ModularEngine::update(const Action& action) {
  if (const auto* p = std::get_if<ColourHintAction>(&action)) {
    if (p->actor != view_.getMyPos()) {
      int after = (p->actor + 3 - view_.getMyPos()) % 4;
      auto comp = computeModular(after);
      checksum_ = 50 + p->colour - comp.first;
      afters_ = comp.second;
    }
  } else if (const auto* p = std::get_if<ValueHintAction>(&action)) {
    if (checksum_ != -1) {
      checksum_ -= 4;
    }
  } else if (const auto* p = std::get_if<PlayAction>(&action)) {
    if (checksum_ != -1) {
      checksum_ -= p->pos;
    }
  } else if (const auto* p = std::get_if<DiscardAction>(&action)) {
    if (checksum_ != -1) {
      checksum_ -= 4;
    }
  }
}

std::optional<small_vector<int, 3>> ModularEngine::toPlay() {
  if (checksum_ == -1) {
    return {};
  }
  small_vector<int, 3> ret;
  ret.push_back(checksum_ % 5);
  for (auto a : afters_) {
    ret.push_back(a);
  }
  checksum_ = -1;
  return ret;
}

int countExtraPlays(
    const std::optional<small_vector<int, 3>>& youAndAfters,
    const small_vector<int, 3>& hintPositions) {
  int hints = std::count_if(
      hintPositions.begin(), hintPositions.end(), [](int p) { return p != 4; });
  if (youAndAfters) {
    hints -= std::count_if(
        youAndAfters->begin() + 1, youAndAfters->end(), [](int p) {
          return p != 4;
        });
  }
  return hints;
}
