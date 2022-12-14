#pragma once

#include <array>
#include <cassert>
#include "Action.h"
#include "Cards.h"
#include "constants.h"
#include "small_vector.h"

struct Token {
  Token(int playerPos, unsigned seed);
  int pos;
  int tok;
};

class GameState {
 public:
  GameState(int numPlayers, unsigned seed);

  // Methods
  Action resolve(int player, const Decision& decision);
  void update(const Action& action);
  bool isOver() const;
  void advanceTurn();

  // Resourcees
  int numHints = STARTING_HINTS;
  int numBombsLeft = STARTING_BOMBS;
  Deck deck;

  // Players
  int numPlayers;
  int currentPlayer = 0;
  const Hand& getHand(const Token& whoami, int pos) const;
  int getHandSize(int pos) const;

  // Table
  std::array<Value, NUM_COLOURS> played;
  std::array<int8_t, NUM_UNIQUE_CARDS> discarded; // count
  // Alternate representations
  int score = 0;
  int maxScore = 25; // goes down when unreachables
  CardSet playableBits;
  CardSet discardableBits;
  CardSet criticalBits;

  // Tracking
  std::vector<Action> actions;
  int turn = 0;
  int lastTurn = 1000; // inclusive

  int seed_;

 private:
  small_vector<Hand, 5> hands_;

  void cardToTable(const Card& card);
  void cardToDiscard(const Card& card);
  void dropAndDraw(int player, int pos);
};
