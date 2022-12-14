#include "NoDrawSim.h"
#include <folly/Synchronized.h>
#include <algorithm>
#include <map>
#include <numeric>

// not technically true, but have three strong preferences:
//  - prefer playing over discarding
//  - prefer discarding discardable over regular over critical
//  - prefer discarding discardable over hinting
//
// Simulate, without drawing, one round of play, optimizing for
//  - score,
//  - low cards played
//  - critical cards played
//  - game making progress

namespace {

struct Key {
  small_vector<Hand, 5> hands;
  std::array<Value, NUM_COLOURS> played;
  int hints;
};

folly::Synchronized<std::map<Key, Decision>> decisions;

// 0: score
// 1: progress
// 2: critsMade
// 3: critsPlayed
// 4: minPile
// 5: hints
std::array<int, 6> computeDecision(
    small_vector<Hand, 5> hands,
    int start,
    std::array<Value, NUM_COLOURS> played,
    int progress,
    int critsMade,
    int critsPlayed,
    int hints) {
  if (start == hands.size()) {
    return;
  }
}

int sim(
    small_vector<Hand, 5> hands,
    std::array<Value, NUM_COLOURS> played,
    int hints) {
  for (auto& hand : hands) {
    std::sort(hand.begin(), hand.end());
  }
  Key k{hands, played, hints};
  SYNCHRONIZED_CONST(decisions) {
    auto dit = decisions.find(k);
    if (dit != decisions.end()) {
      return dit->second;
    }
  }

  auto d = compute(hands, 0, played, hints);

  SYNCHRONIZED(decisions) {
    decisions[k] = d.decision;
  }
  return d.decision;
}
} // namespace

Decision noDrawSimulation(const GameState& game, const Token& omni) {}
