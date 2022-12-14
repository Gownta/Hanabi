#include "Cards.h"
#include <glog/logging.h>
#include <algorithm>
#include <cassert>
#include <random>
#include "constants.h"

int Card::index() const {
  return colour * 5 + value - 1;
}
bool operator==(const Card& lhs, const Card& rhs) {
  return lhs.value == rhs.value && lhs.colour == rhs.colour;
}
bool operator<(const Card& lhs, const Card& rhs) {
  return lhs.index() < rhs.index();
}

CardSet::CardSet(Value v) {
  for (const auto& c : uniqueCards()) {
    if (c.value == v) {
      add(c);
    }
  }
}

void CardSet::add(const Card& c) {
  bits_ |= 1 << c.index();
}

void CardSet::remove(const Card& c) {
  bits_ &= ~(1 << c.index());
}

bool CardSet::has(const Card& c) const {
  return bits_ & (1 << c.index());
}

Deck::Deck(unsigned seed) {
  deck_ = allCards();
  std::mt19937_64 rng(seed);
  std::shuffle(deck_.begin(), deck_.end(), rng);
}

bool Deck::empty() const {
  return deck_.empty();
}

int Deck::size() const {
  return deck_.size();
}

Card Deck::draw() {
  assert(!deck_.empty());
  return deck_.pop_back();
}

const small_vector<Card, NUM_UNIQUE_CARDS>& uniqueCards() {
  static small_vector<Card, NUM_UNIQUE_CARDS> uc = []() {
    small_vector<Card, NUM_UNIQUE_CARDS> ret;
    for (auto c : Colours) {
      for (auto v : Values) {
        ret.push_back(Card(c, v));
      }
    }
    return ret;
  }();
  return uc;
}

const small_vector<Card, DECK_SIZE>& allCards() {
  static small_vector<Card, DECK_SIZE> ac = []() {
    small_vector<Card, DECK_SIZE> ret;
    for (auto c : Colours) {
      for (auto v : Values) {
        for (int j = 0; j < CARDS_PER_VALUE[v]; ++j) {
          ret.push_back(Card(c, v));
        }
      }
    }
    return ret;
  }();
  return ac;
}
