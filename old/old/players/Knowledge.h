#pragma once

#include "Action.h"
#include "GameState.h"

void knowledgeInit();

// What does a player know about their hand
// Possibly as perceived by someone else (who's view we use)
//
// Common knowledge is what everyone can deduce. It is based on
// hints given, and cards played or discarded
//
// Uncommon knowledge is known to the hand-holder, but the fact that the
// hand-holders knows so is not necessarily known to all others
//
// Crucial cards are cards for which only 1 is undiscarded.
// Note: all 5s are crucial
class Knowledge {
 public:
  Knowledge(View v, int p);
  void update(const Action& action);
  void startGame();

  // Indexed by position in hand
  small_vector<CardBits, 4> common;
  small_vector<CardBits, 4> uncommon;

  // Uses uncommon knowledge
  bool isColour(int pos, Colour c);
  bool isValue(int pos, Value v);
  bool isPlayable(int pos);
  bool isDiscardable(int pos);
  bool isCrucial(int pos);
  int numPlayable();
  int numDiscardable();
  int numCrucial();

 private:
  View view_;
  int pos_;
  CardBits playable_;
  CardBits played_;
  CardBits crucial_;
  CardBits unreachable_;
};
