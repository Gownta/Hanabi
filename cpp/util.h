#pragma once

#include "GameState.h"

int numPlayable(Hand h, const Played& played);

// number of (reachable) non-5 criticals
// number of (reachable) 5s left to play
// number of (reachable) cards behind a critical
// number of unreachable cards
// std::array<int8_t, 4> analyzeCriticals(
//    const std::array<Value, 5>& played,
//    const std::array<int8_t, 26>& discarded);

struct ColourPermutationResult {
  bool isIdentity;
  Perm permutation;
  Perm inverse;
};
ColourPermutationResult getPermutation(const PublicGameState& g);

Card permute(const Perm& perm, Card c);
Hand permute(const Perm& perm, Hand h);
