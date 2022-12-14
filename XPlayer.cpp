#include "Cards.h"
#include "IPlayer.h"

#include <memory>

struct HandKnowledge {
  std::vector<Hand> handPossibilities;
  std::array<CardSet, 5> cardPossibilities;
};

int waysToChecksumHint(
    const HandKnowledge& common,
    const PublicGameState& /*g*/) {
  /**
   * Can always give a player a colour or a number hint.
   * If they have at least 4 cards, then their hand contains at least two
   * distinct cards, and so there are at least 3 hints. Can common-knowledge
   * this by saying "colour of first card", "number of first card", or "any hint
   * not including the first card".
   */

  auto handSize = common.handPossibilities.front().size();
  if (handSize >= 4) {
    return 3;
  }
  return 2;
}

int checksumOfHand(
    const Hand& hand,
    const HandKnowledge& common,
    const PublicGameState& g,
    int modulus) {
  return 0;
}

/**
 * When checksum-hinting, everyone must agree on:
 *
 *  1. How to map a hint to and from an index, and
 *  2. How to map a hand to and from an index
 */
struct HintOptions {
  // How many publicly-known ways can the hand be hinted?
  int ways;

  //
};

struct Count {
  std::array<int8_t, 26> card;
  std::array<int8_t, 5> colour;
  std::array<int8_t, 6> value;

  Count() {
    card.fill(0);
    colour.fill(0);
    value.fill(0);
  }
};

/**
 * There are 2*(n-1) main hints to give (colour v number to each other player)
 * with a further <=m possible variants (based on which colour/value).
 *
 * Discarding has <=m possible variants.
 *
 * Each player has a set of common-knowledge hand possibilities.
 */

class XPlayer : public IPlayer {
  // inherits protected member `GameView v`
 public:
  XPlayer(GameView v) : IPlayer(v) {}

  Decision act() override {
    return Decision::play(0);
  }

  void inform(const Action&) override {}

 private:
  // common knowledge
  std::array<HandKnowledge, 5> k;
  std::array<int, 5> playOrders;
  Count count;
};

std::unique_ptr<IPlayer> makeXPlayer(GameView v) {
  return std::make_unique<XPlayer>(v);
}
