#include "util.h"
#include <algorithm>
#include "Cards.h"

int numPlayable(Hand h, const Played& played) {
  int ret = 0;
  for (auto c : h) {
    ret += played.isPlayable(c);
  }
  return ret;
}

/*
// number of (reachable) non-5 criticals
// number of (reachable) 5s left to play
// number of (reachable) cards behind a critical
// number of unreachable cards
std::array<int8_t, 4> analyzeCriticals(
    const std::array<Value, 5>& played,
    const std::array<int8_t, 26>& discarded) {
  std::array<int8_t, 4> ret;
  ret.fill(0);
  for (auto colour : colours()) {
    bool critExists = false;
    for (int value = played[colour] + 1; value <= 5; ++value) {
      int nd = discarded[Card(colour, value).getIdx()];
      if (nd == numCardsPerValue()[value]) {
        ret[3] += 6 - value;
        break;
      }
      ret[2] += critExists;
      if (nd == numCardsPerValue()[value] - 1) {
        if (value == 5) {
          ret[1]++;
        } else {
          ret[0]++;
          critExists = true;
        }
      }
    }
  }
  return ret;
}*/

ColourPermutationResult getPermutation(const PublicGameState& g) {
  ColourPermutationResult ret;

  ret.inverse = {0, 1, 2, 3, 4};
  ret.permutation = ret.inverse;
  std::stable_sort(
      ret.inverse.begin(), ret.inverse.end(), [&](Colour c1, Colour c2) {
        return g.played(c1) < g.played(c2);
      });

  ret.isIdentity = ret.inverse == ret.permutation;
  if (ret.isIdentity) {
    return ret;
  }

  for (int i = 0; i < 5; ++i) {
    ret.permutation[ret.inverse[i]] = i;
  }
  return ret;
}

Card permute(const Perm& perm, Card c) {
  return Card(perm[c.colour()], c.value());
}

Hand permute(const Perm& perm, Hand h) {
  Hand ret;
  for (auto c : h) {
    ret.add(permute(perm, c));
  }
  return ret;
}
