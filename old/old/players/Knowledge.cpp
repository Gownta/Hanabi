#include "Knowledge.h"

namespace {
static std::array<CardBits, NUM_COLOURS> ColourBits;
static std::array<CardBits, NUM_VALUES + 1> ValueBits;
} // namespace

void knowledgeInit() {
  ColourBits.fill(0);
  ValueBits.fill(0);
  for (auto c : Colours) {
    for (auto v : Values) {
      int mask = 1 << Card(c, v).index();
      ColourBits[c] |= mask;
      ValueBits[v] |= mask;
    }
  }
}

Knowledge::Knowledge(View v, int p) : view_(v), pos_(p) {
  playable_ ValueBits[1];
  played_ = 0;
  crucial_ = ValueBits[5];
  unreachable_ = 0;
}

bool Knowledge::isColour(int pos, Colour c) const {
  return (uncommon[pos] & ~ColourBits[c]) == 0;
}

bool Knowledge::isValue(int pos, Value v) const {
  return (uncommon[pos] & ~ValueBits[v]) == 0;
}

bool Knowledge::isPlayable(int pos) const {
  return (uncommon[pos] & ~playable_) == 0;
}

bool Knowledge::isDiscardable(int pos) const {
  return (uncommon[pos] & ~(played_ | unreachable_)) == 0;
}

bool Knowledge::isCrucial(int pos) const {
  return (uncommon[pos] & ~crucial_) == 0;
}

int Knowledge::numPlayable() const {
  int ret = 0;
  for (int i = 0; i < view_.getHandSize(pos_); ++i) {
    ret += isPlayable(i);
  }
  return ret;
}

int Knowledge::numDiscardable() const {
  int ret = 0;
  for (int i = 0; i < view_.getHandSize(pos_); ++i) {
    ret += isDiscardable(i);
  }
  return ret;
}

int Knowledge::numCrucial() const {
  int ret = 0;
  for (int i = 0; i < view_.getHandSize(pos_); ++i) {
    ret += isCrucial(i);
  }
  return ret;
}

void Knowledge::update(const Action& action) {
  if (const auto* p = std::get_if<PlayAction>(&action)) {
    if (p->actor == pos_) {
      int cpos = p->pos;
      const Card& card = p->card;

      // remove card
      uncommon.erase(pos);

      // see if there is a new card
      int newHandSize = view_.getHandSize(pos_);
      if (newHandSize != uncommon.size()) {
        assert(newHandSize == uncommon.size() + 1);
        uncommon.push_back(0);
      }

      // remove all common knowledge of possibilities
      // if I know of certain cards in my hand, I might be able to further
      // remove possibilities
      //
      // Also update playable, played, and crucial
    }
  }
}

void Knowledge::startGame() {
  return 5; // TODO implement
}
