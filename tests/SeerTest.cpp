#include <folly/Optional.h>
#include <gtest/gtest.h>
#include <variant>
#include "experimental/njormrod/hanabi/SeerExecutor.h"

using namespace seer;

class SeerTest : public ::testing::Test {
 public:
  void SetUp() override {
    initCards(false);
    ng.emplace(4, 0);

    while (ng->deck.size() > 4) {
      auto c = ng->deck.draw();
      ng->deckSize--;
    }
    for (int i = 0; i < ng->numPlayers; ++i) {
      ng->hands[i] = Hand();
    }
  }

  folly::Optional<NaturesGameState> ng;
};

TEST_F(SeerTest, get_options) {
  Played played{0, 1, 2, 3, 5};

  { // empty hand
    Hand h;
    auto o1 = getOptions(h, played, 1);
    ASSERT_EQ(o1.size(), 1);
    EXPECT_EQ(o1[0], SeerDecision::Hint());

    auto o2 = getOptions(h, played, 0);
    EXPECT_EQ(o2.size(), 0);
  }

  { // dupe playable, regular discard
    Hand h;
    h.add(BLUE, 1);
    h.add(BLUE, 1);
    h.add(GREEN, 2);
    h.add(RED, 1);
    auto o = getOptions(h, played, 1);
    ASSERT_EQ(o.size(), 4);
    EXPECT_EQ(o[0], SeerDecision::Play(Card(BLUE, 1)));
    EXPECT_EQ(o[1], SeerDecision::Play(Card(GREEN, 2)));
    EXPECT_EQ(o[2], SeerDecision::Discard(Card(RED, 1)));
    EXPECT_EQ(o[3], SeerDecision::Hint());
  }

  { // dupe discardable
    Hand h;
    h.add(BLUE, 4);
    h.add(BLUE, 4);
    auto o = getOptions(h, played, 1);
    ASSERT_EQ(o.size(), 2);
    EXPECT_EQ(o[0], SeerDecision::Discard(Card(BLUE, 4)));
    EXPECT_EQ(o[1], SeerDecision::Hint());
  }

  { // no discardables
    Hand h;
    h.add(BLUE, 1);
    h.add(BLUE, 5);
    h.add(RED, 5);
    auto o = getOptions(h, played, 1);
    ASSERT_EQ(o.size(), 5);
    EXPECT_EQ(o[0], SeerDecision::Play(Card(BLUE, 1)));
    EXPECT_EQ(o[1], SeerDecision::Hint());
    EXPECT_EQ(o[2], SeerDecision::Discard(Card(BLUE, 1)));
    EXPECT_EQ(o[3], SeerDecision::Discard(Card(BLUE, 5)));
    EXPECT_EQ(o[4], SeerDecision::Discard(Card(RED, 5)));
  }

  { // not allowed bad discardables
    Hand h;
    h.add(BLUE, 5);
    auto o1 = getOptions(h, played, 0);
    auto o2 = getOptions(h, played, 0, false);
    EXPECT_EQ(o1.size(), 1);
    EXPECT_EQ(o2.size(), 0);
  }
}

TEST_F(SeerTest, translate) {
  GameState g(4);

  // score is {1 0 0 4 0}
  g.play(BLUE);
  g.play(WHITE);
  g.play(WHITE);
  g.play(WHITE);
  g.play(WHITE);
  g.currentPlayer = 2;
  g.hands[2].add(Card(2, 3));
  g.hands[2].add(Card(0, 2)); // playable in translation

  {
    auto psd = SeerDecision::Play(Card(3, 2));
    auto pd = translate(psd, g, true);
    ASSERT_EQ(pd.type, PLAY);
    EXPECT_EQ(pd.pos, 1);
  }
  {
    auto bsd = SeerDecision::Play(Card(0, 2));
    auto bd = translate(bsd, g, false);
    ASSERT_EQ(bd.type, PLAY);
    EXPECT_EQ(bd.pos, 1);
  }
  {
    auto dsd = SeerDecision::Discard(Card(1, 3));
    auto dd = translate(dsd, g, true);
    ASSERT_EQ(dd.type, DISCARD);
    EXPECT_EQ(dd.pos, 0);
  }
  {
    g.hands[0].add(Card(2, 3));
    g.hands[1].add(Card(2, 3));
    g.hands[3].add(Card(2, 3));
    auto hsd = SeerDecision::Hint();
    auto hd = translate(hsd, g, true);
    ASSERT_EQ(hd.type, HINT_VALUE);
    EXPECT_EQ(hd.value, 3);
  }
}

TEST_F(SeerTest, eg_ctor) {
  auto g = SeerEndGameState::mk(*ng);

  EXPECT_EQ(g.deckSize, 4);
  EXPECT_EQ(g.deck[4], Card());

  auto gs = g.serialize();
  g.reorder();
  EXPECT_EQ(g.serialize(), gs);
  g.yellowify();
  EXPECT_EQ(g.serialize(), gs);
  g.rotate();
  EXPECT_EQ(g.serialize(), gs);
  g.sort();
  EXPECT_EQ(g.serialize(), gs);
}

TEST_F(SeerTest, reorder) {
  auto g = SeerEndGameState::mk(*ng);

  g.play(BLUE);
  g.play(BLUE);
  g.play(GREEN);
  g.play(GREEN);

  g.hands[0].add(Card(1, 1));
  g.discard(Card(3, 2));
  g.discard(Card(3, 2));
  g.discard(Card(3, 2));
  g.discard(Card(3, 2));
  g.deckSize = 1;
  g.deck[0] = Card(4, 4);

  g.reorder();
  EXPECT_EQ(g.played(0), 0);
  EXPECT_EQ(g.played(4), 2);
  EXPECT_EQ(g.hands[0][0], Card(4, 1));
  EXPECT_EQ(g.numDiscarded(Card(3, 2)), 0);
  EXPECT_EQ(g.numDiscarded(Card(1, 2)), 4);
  EXPECT_EQ(g.deck[0], Card(2, 4));
}

TEST_F(SeerTest, yellowify) {
  auto g = SeerEndGameState::mk(*ng);

  g.play(BLUE);
  g.play(BLUE);
  g.play(GREEN);
  g.play(GREEN);
  g.hands[0].add(Card(0, 1));
  g.hands[0].add(Card(0, 3));
  g.discard(Card(0, 2), 8);
  g.discard(Card(0, 5), 9);
  g.deckSize = 2;
  g.deck[0] = Card(1, 1);
  g.deck[1] = Card(2, 1);

  g.yellowify();
  EXPECT_EQ(g.hands[0][0], Card(4, 1));
  EXPECT_EQ(g.hands[0][1], Card(0, 3));
  EXPECT_EQ(g.numDiscarded(Card(4, 1)), 8);
  EXPECT_EQ(g.numDiscarded(Card(0, 5)), 9);
  EXPECT_EQ(g.deck[0], Card(4, 1));
  EXPECT_EQ(g.deck[1], Card(2, 1));
}

TEST_F(SeerTest, rotate) {
  auto g = SeerEndGameState::mk(*ng);

  g.hands[0].add(Card(1, 1));
  g.currentPlayer = 2;
  g.rotate();
  EXPECT_EQ(g.currentPlayer, 0);
  EXPECT_EQ(g.hands[2][0], Card(1, 1));
}

TEST_F(SeerTest, sort) {
  auto g = SeerEndGameState::mk(*ng);

  g.hands[0].add(Card(2, 2));
  g.hands[0].add(Card(1, 1));
  g.deckSize = 2;
  g.deck[0] = Card(4, 4);
  g.deck[1] = Card(3, 3);
  g.sort();
  EXPECT_EQ(g.hands[0][0], Card(1, 1));
  EXPECT_EQ(g.hands[0][1], Card(2, 2));
  EXPECT_EQ(g.deck[0], Card(3, 3));
  EXPECT_EQ(g.deck[1], Card(4, 4));
}

TEST_F(SeerTest, upAndAdv) {
  auto g = SeerEndGameState::mk(*ng);
  EXPECT_EQ(g.numPlayers, 4);

  g.hands[0].add(Card(0, 1));
  auto d1 = SeerDecision::Play(Card(0, 1));
  Card replace(0, 5);
  g.deck[1] = replace;
  EXPECT_EQ(g.deckSize, 4);
  g.updateAndAdvance(d1, 1);
  EXPECT_EQ(g.deckSize, 3);
  EXPECT_EQ(g.hands[3][0].value(), replace.value());
  EXPECT_EQ(g.deck[g.deckSize], Card());
  auto gs = g.serialize();
  g.reorder();
  g.yellowify();
  g.rotate();
  g.sort();
  EXPECT_EQ(g.serialize(), gs);
}
