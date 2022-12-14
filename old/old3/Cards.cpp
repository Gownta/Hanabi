#include "Cards.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iterator>
#include <mutex>
#include <random>
#include <utility>
#include "common/logging/logging.h"

DEFINE_string(hands_file, "", "");
DEFINE_string(hands_output_file, "", "");

namespace detail {
namespace {
std::array<std::pair<Colour, Value>, 26> CCV;
constexpr std::array<int8_t, 6> CardsPerValue{0, 3, 2, 2, 2, 1};
std::array<Card, 25> UniqueCards;
std::array<Card, 50> AllCards;
constexpr std::array<Colour, 5> AllColours{BLUE, GREEN, RED, WHITE, YELLOW};
constexpr std::array<Value, 5> AllValues{1, 2, 3, 4, 5};
std::array<int32_t, 6> CardSetBitsPerValue;

struct HandInfo {
  std::array<Card, 5> cards;
  int8_t size;

  Hand sorted;
  CardSet cardSet;
  std::array<Hand, 5> rm; // transforms hand H into H\pos
};
constexpr std::array<int, 7>
    HandsSizeOffset{0, 1, 26, 651, 16276, 406901, 10172526};
std::array<HandInfo, HandsSizeOffset[5]> Hands; // all hands of size at most 4

void initCCV() {
  CCV[0] = {0, 0};
  int ucidx = 0;
  int acidx = 0;
  for (auto c : AllColours) {
    for (auto v : AllValues) {
      Card card(c, v);
      CCV[card.getIdx()] = {c, v};
      UniqueCards[ucidx++] = card;
      for (int j = 0; j < CardsPerValue[v]; ++j) {
        AllCards[acidx++] = card;
      }
    }
  }
}
void initCSBPV() {
  for (auto v : AllValues) {
    CardSet cardSet;
    for (auto c : AllColours) {
      cardSet.add(Card(c, v));
    }
    CardSetBitsPerValue[v] = cardSet.getIdx();
  }
}

int getHandIdx(const std::array<Card, 5>& cards, int sz) {
  int offset = 0;
  for (int i = 0; i < sz; ++i) {
    offset *= 25;
    offset += cards[i].getIdx() - 1;
  }
  return offset + HandsSizeOffset[sz];
}

void initHand(const std::array<Card, 5>& cards, int sz) {
  auto& hi = Hands[getHandIdx(cards, sz)];

  hi.cards = cards;
  for (int i = sz; i < hi.cards.size(); ++i) {
    hi.cards[i] = Card();
  }
  hi.size = sz;

  auto sorted = cards;
  std::sort(sorted.begin(), sorted.begin() + sz);
  hi.sorted = Hand(getHandIdx(sorted, sz));

  for (int i = 0; i < sz; ++i) {
    hi.cardSet.add(cards[i]);
  }

  if (sz > 0) {
    for (int i = 0; i < sz; ++i) {
      auto toRm = cards;
      for (int j = i; j + 1 < sz; ++j) {
        std::swap(toRm[j], toRm[j + 1]);
      }
      hi.rm[i] = Hand(getHandIdx(toRm, sz - 1));
    }
  }
}

void initHands(bool incl5) {
  incl5 = false;
  if (FLAGS_hands_file != "") {
    std::ifstream input(FLAGS_hands_file, std::ios::binary);
    input.read(reinterpret_cast<char*>(&Hands), sizeof(Hands));
    return;
  }
  std::array<Card, 5> cards;
  initHand(cards, 0);
  for (auto c1 : UniqueCards) {
    cards[0] = c1;
    initHand(cards, 1);
    for (auto c2 : UniqueCards) {
      cards[1] = c2;
      initHand(cards, 2);
      for (auto c3 : UniqueCards) {
        cards[2] = c3;
        initHand(cards, 3);
        for (auto c4 : UniqueCards) {
          cards[3] = c4;
          initHand(cards, 4);
          if (incl5) {
            for (auto c5 : UniqueCards) {
              cards[4] = c5;
              initHand(cards, 5);
            }
          }
        }
      }
    }
  }

  if (FLAGS_hands_output_file != "") {
    std::ofstream output(FLAGS_hands_output_file, std::ios::binary);
    output.write(reinterpret_cast<const char*>(&Hands), sizeof(Hands));
  }
}
} // namespace
} // namespace detail

using namespace detail;

void initCards(bool incl5) {
  static std::once_flag f;
  std::call_once(f, [incl5] {
    initCCV();
    initCSBPV();
    initHands(incl5);
  });
}

Card::Card() : idx_(0) {}
Card::Card(Colour c, Value v) : idx_(5 * c + v) {}

Colour Card::colour() const {
  return CCV[idx_].first;
}

Value Card::value() const {
  return CCV[idx_].second;
}

int8_t Card::getIdx() const {
  return idx_;
}

bool operator==(Card lhs, Card rhs) {
  return lhs.getIdx() == rhs.getIdx();
}

bool operator!=(Card lhs, Card rhs) {
  return !(lhs == rhs);
}

bool operator<(Card lhs, Card rhs) {
  return lhs.getIdx() < rhs.getIdx();
}

const std::array<Card, 25>& uniqueCards() {
  return UniqueCards;
}
const std::array<Card, 50>& allCards() {
  return AllCards;
}
const std::array<Colour, 5>& colours() {
  return AllColours;
}
const std::array<Value, 5>& values() {
  return AllValues;
}
const std::array<int8_t, 6>& numCardsPerValue() {
  return CardsPerValue;
}

CardSet::CardSet() : bits_(0) {}

CardSet::CardSet(Value v) : bits_(CardSetBitsPerValue[v]) {}

void CardSet::add(Card c) {
  bits_ |= (1 << c.getIdx());
}

void CardSet::remove(Card c) {
  bits_ &= ~(1 << c.getIdx());
}

bool CardSet::has(Card c) const {
  return bits_ & (1 << c.getIdx());
}

int32_t CardSet::getIdx() const {
  return bits_;
}

Hand::Hand() : idx_(0) {}

Hand::Hand(int32_t idx) : idx_(idx) {}

void Hand::add(Card c) {
  auto sz = size();
  assert(sz < 5);
  idx_ = (idx_ - HandsSizeOffset[sz]) * 25 + c.getIdx() - 1 +
      HandsSizeOffset[sz + 1];
}

void Hand::add(Colour c, Value v) {
  add(Card(c, v));
}

void Hand::erase(int pos) {
  idx_ = Hands[idx_].rm[pos].idx_;
}

int Hand::size() const {
  return Hands[idx_].size;
}

Card Hand::operator[](int pos) const {
  return Hands[idx_].cards[pos];
}

const Card* Hand::begin() const {
  return &Hands[idx_].cards[0];
}

const Card* Hand::end() const {
  return &Hands[idx_].cards[0] + Hands[idx_].size;
}

int32_t Hand::getIdx() const {
  return idx_;
}

Hand Hand::sorted() const {
  return Hands[idx_].sorted;
}

CardSet Hand::getCardSet() const {
  return Hands[idx_].cardSet;
}

bool operator==(Hand lhs, Hand rhs) {
  return lhs.getIdx() == rhs.getIdx();
}

bool operator!=(Hand lhs, Hand rhs) {
  return !(lhs == rhs);
}

bool operator<(Hand lhs, Hand rhs) {
  return lhs.getIdx() < rhs.getIdx();
}

constexpr int NumHandIndexes() {
  return HandsSizeOffset[6];
}

Deck::Deck(unsigned seed) {
  deck_ = allCards();
  std::mt19937_64 rng(seed);
  std::shuffle(deck_.begin(), deck_.end(), rng);
  size_ = 50;
}

bool Deck::empty() const {
  return size_ == 0;
}

int Deck::size() const {
  return size_;
}

Card Deck::draw() {
  assert(!empty());
  size_--;
  auto ret = deck_[size_];
  deck_[size_] = Card();
  return ret;
}
