#pragma once

#include <vector>
#include "Action.h"
#include "GameState.h"

namespace seer {

struct SeerDecision {
  enum What {
    HINT,
    DISCARD,
    PLAY,
  };

  static SeerDecision Play(Card c);
  static SeerDecision Hint();
  static SeerDecision Discard(Card c);

  What what;
  Card card;
};
bool operator==(const SeerDecision& lhs, const SeerDecision& rhs);

// returns plays, safe discard, hint, (unsafe discards)
std::vector<SeerDecision> getOptions(
    Hand h,
    const Played& played,
    int numHints,
    bool allowBadDiscards = true);

Decision translate(
    const SeerDecision& action,
    const GameState& g,
    bool underIsomorphism);

class SeerEndGameState : public GameState {
  SeerEndGameState(const NaturesGameState& g);

 public:
  static SeerEndGameState mk(const NaturesGameState& g);
  SeerEndGameState(const SeerEndGameState& g) noexcept;

  // Isomorphisms, all idempotent
  void reorder(); // scores are increasing by colour
  void yellowify(); // discardable cards become yellow 1s
  void rotate(); // so currentPlayer == 0
  void sort(); // deck and hands sorted

  std::string serialize() const;

  void updateAndAdvance(const SeerDecision& decision, int nextCardIdx);

  std::array<Card, 5> deck;
};

// assumption: plays are strictly better than anything else
// known: discarding discardable strictly better than hinting and undiscardables
SeerDecision noDrawMaximize(const NaturesGameState& g);

} // namespace seer
