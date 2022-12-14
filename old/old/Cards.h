#pragma once

#include <array>
#include "constants.h"
#include "small_vector.h"

enum Colour : int8_t {
  BLUE,
  GREEN,
  RED,
  WHITE,
  YELLOW,
};
constexpr std::array<Colour, 5> Colours{BLUE, GREEN, RED, WHITE, YELLOW};

using Value = int8_t;
constexpr std::array<Value, 5> Values{1, 2, 3, 4, 5};

struct Card {
  Colour colour;
  Value value;
  Card(Colour c, Value v) : colour(c), value(v) {}

  // [0, 25)
  int index() const;
};
static_assert(sizeof(Card) == 2);
bool operator==(const Card& lhs, const Card& rhs);
bool operator<(const Card& lhs, const Card& rhs);

class CardSet {
 public:
  CardSet() = default;
  CardSet(Value v);

  void add(const Card& c);
  void remove(const Card& c);
  bool has(const Card& c) const;

 private:
  int32_t bits_ = 0; // at least 25 bits
};

using Hand = small_vector<Card, 5>;

class Deck {
 public:
  Deck(unsigned seed);
  bool empty() const;
  int size() const;
  Card draw();

 private:
  small_vector<Card, DECK_SIZE> deck_;
};

const small_vector<Card, NUM_UNIQUE_CARDS>& uniqueCards();
const small_vector<Card, DECK_SIZE>& allCards();
