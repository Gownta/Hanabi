#pragma once

#include <array>
#include <initializer_list>
#include "Action.h"
#include "Cards.h"

class Played {
 public:
  Played();
  Played(std::initializer_list<Value> l);

  Value played(Colour c) const;
  Value played(Card c) const;
  int numPlayed() const;

  bool isPlayable(Card c) const;
  bool isDiscardable(Card c) const;
  CardSet getPlayables() const;
  CardSet getDiscardables() const;

 protected:
  void play(Colour c);
  void play(Card c);

  // bool operator==(const Played& other) const;

 private:
  std::array<Value, 5> played_;
  CardSet playable_;
  CardSet discardable_;
};

class Discarded {
 public:
  Discarded();

  int8_t numDiscarded() const;
  int8_t numDiscarded(Card c) const;

 protected:
  void discard(Card c, int count = 1);

 private:
  int8_t total_;
  std::array<int8_t, 26> discarded_;
};

struct PublicGameState : public Played, public Discarded {
  PublicGameState(int numPlayers);

  void playCard(Card card);
  void discardCard(Card card);
  bool isOver() const;
  int score() const;

  int8_t numPlayers;
  int8_t currentPlayer = 0;
  int8_t numHints = 8;
  int8_t numBombsLeft = 3;
  int8_t deckSize = 50;
  int8_t turn = 0;
  int8_t actionsLeft = -1; // -1 when deckSize > 0
};

struct GameState : public PublicGameState {
  GameState(int numPlayers);

  Action resolve(int player, const Decision& decision) const;
  void dropAndDraw(int player, int pos, Card draw); // Card() means no draw
  void advanceTurn(); // does not decrement actionsLeft

  std::array<Hand, 5> hands;
};

struct NaturesGameState : public GameState {
  NaturesGameState(int numPlayers, unsigned seed);

  void update(const Action& action); // does not advance the turn

  Deck deck;
  unsigned seed;
};

/*
struct GameView {
  GameView(const GameState* g, int player);

  const PublicGameState* operator->() const;

  // 0 is self, which is disallowed
  Hand operator[](int pos) const;

 private:
  const GameState* g;
  int myPos;
};
*/
