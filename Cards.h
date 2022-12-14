#pragma once

#include <array>
#include <vector>

void initCards(bool include_5_card_hands = true);

using Colour = int8_t;
enum Colour_ : int8_t {
  BLUE = 0,
  GREEN,
  RED,
  WHITE,
  YELLOW,
};
using Value = int8_t;

/**
 * Perm is a permutation of the colours.
 */
using Perm = std::array<Colour, 5>;

class Card {
 public:
  Card();
  Card(Colour c, Value v);

  Colour colour() const;
  Value value() const;

  // [0,26). 0 means "not really a card"
  int8_t getIdx() const;

 private:
  int8_t idx_;
};
bool operator==(Card lhs, Card rhs);
bool operator!=(Card lhs, Card rhs);
bool operator<(Card lhs, Card rhs);

const std::array<Card, 25>& uniqueCards();
const std::array<Card, 50>& allCards();
const std::array<Colour, 5>& colours();
const std::array<Value, 5>& values();
const std::array<int8_t, 6>& numCardsPerValue();

class CardSet {
 public:
  CardSet();
  CardSet(Value v);

  void add(Card c);
  void remove(Card c);
  bool has(Card c) const;

  int32_t getIdx() const;

  void permute(const Perm& p);

 private:
  int32_t bits_;
};

class Hand {
 public:
  Hand();
  explicit Hand(int32_t idx);

  void add(Card c);
  void add(Colour c, Value v);
  void erase(int pos);

  int size() const;
  Card operator[](int pos) const;
  const Card* begin() const;
  const Card* end() const;

  int32_t getIdx() const;
  Hand sorted() const;
  CardSet getCardSet() const;

 private:
  int32_t idx_;
};
bool operator==(Hand lhs, Hand rhs);
bool operator!=(Hand lhs, Hand rhs);
bool operator<(Hand lhs, Hand rhs);
constexpr int NumHandIndexes();

// May contain illegal hands, such as by having two blue fives
std::vector<Hand> allHands(int handSize);

class Deck {
 public:
  Deck(unsigned seed);
  bool empty() const;
  int size() const;
  Card draw();

 private:
  int8_t size_;
  std::array<Card, 50> deck_;
};
