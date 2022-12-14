#include "Brute.h"

Brute::Brute(int p, Token omni) : pos(p), tok(omni) {}

std::vector<Decision> Brute::actions(const GameState& game) const {
  // For a seer, it is strictly better to play a non-5 than to discard
  // For a seer, it is strictly better to discard a discardable card

  std::vector<Decision> ret;
  ret.reserve(9);

  const auto& hand = game.getHand(tok, pos);
  bool playNon5 = false;
  for (int i = 0; i < hand.size(); ++i) {
    if (game.playableBits.has(hand[i])) {
      if (hand[i].value != 5) {
        playNon5 = true;
      }
      ret.push_back(PlayDecision{i});
    }
  }

  if (game.numHints < STARTING_HINTS && !playNon5) {
    auto rit = ret.end();
    for (int i = 0; i < hand.size(); ++i) {
      bool d = game.discardableBits.has(hand[i]);
      if (d) {
        ret.erase(rit, ret.end());
        ret.push_back(DiscardDecision{i});
        break;
      }
      ret.push_back(DiscardDecision{i});
    }
  }

  if (game.numHints > 0) {
    int who = (pos + 1) % 4;
    Value what = game.getHand(tok, who)[0].value;
    ret.push_back(ValueHintDecision{who, what});
  }

  return ret;
}
