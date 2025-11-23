#include <gtest/gtest.h>
#include <set>
#include "experimental/njormrod/hanabi/GameState.h"

class GameTest : public ::testing::Test {
 public:
  void SetUp() override {
    initCards(false);

    htest.add(Card(0, 1));
    htest.add(Card(0, 3));
    htest.add(Card(1, 1));
    htest.add(Card(1, 2));
  }
  // b1 b3 g1 g2
  Hand htest;
};

// Long test, only run occasionally
#if false
TEST(GameTest2345, handSize) {
  initCards(true);

  NaturesGameState g2(2, 0), g3(3, 0), g4(4, 0), g5(5, 0);
  auto e = [](const NaturesGameState& g, int np) {
    for (int i = 0; i < 5; ++i) {
      int expected = i >= np ? 0 : np >= 4 ? 4 : 5;
      EXPECT_EQ(g.hands[i].size(), expected);
    }
  };
  e(g2, 2);
  e(g3, 3);
  e(g4, 4);
  e(g5, 5);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Played
////////////////////////////////////////////////////////////////////////////////

TEST_F(GameTest, played_ctord) {
  Played p;
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(p.played(i), 0);
  }
}

TEST_F(GameTest, played_ctoril) {
  Played p{0, 1, 3, 2, 0};
  EXPECT_EQ(p.played(0), 0);
  EXPECT_EQ(p.played(1), 1);
  EXPECT_EQ(p.played(2), 3);
  EXPECT_EQ(p.played(3), 2);
  EXPECT_EQ(p.played(4), 0);
}

TEST_F(GameTest, played_getters) {
  Played p{0, 2, 3, 1, 1};

  EXPECT_TRUE(p.isPlayable(Card(BLUE, 1)));
  EXPECT_FALSE(p.isPlayable(Card(BLUE, 2)));
  EXPECT_TRUE(p.isPlayable(Card(GREEN, 3)));

  EXPECT_TRUE(p.isDiscardable(Card(RED, 2)));
  EXPECT_TRUE(p.isDiscardable(Card(RED, 3)));
  EXPECT_FALSE(p.isDiscardable(Card(RED, 4)));
}

TEST_F(GameTest, played_pd) {
  Played p1;

  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(p1.getPlayables().has(Card(i, 1)));
  }
  EXPECT_EQ(p1.getDiscardables().getIdx(), 0);

  Played p2{1, 0, 3, 5, 4};

  p1.play(Card(0, 1));
  p1.play(Card(2, 1));
  p1.play(Card(2, 2));
  p1.play(Card(2, 3));
  p1.play(Card(3, 1));
  p1.play(Card(3, 2));
  p1.play(Card(3, 3));
  p1.play(Card(3, 4));
  p1.play(Card(3, 5));
  p1.play(Card(4, 1));
  p1.play(Card(4, 2));
  p1.play(Card(4, 3));
  p1.play(Card(4, 4));

  EXPECT_EQ(p1.getPlayables().getIdx(), p2.getPlayables().getIdx());
  EXPECT_EQ(p1.getDiscardables().getIdx(), p2.getDiscardables().getIdx());
  auto ps = p1.getPlayables();
  auto ds = p1.getDiscardables();
  for (int c = 0; c < 5; ++c) {
    for (int v = 1; v <= 5; ++v) {
      EXPECT_EQ(ps.has(Card(c, v)), p1.played(c) + 1 == v);
      EXPECT_EQ(ds.has(Card(c, v)), p1.played(c) >= v);
    }
  }
}

TEST_F(GameTest, played_play) {
  Played p;
  EXPECT_EQ(p.played(BLUE), 0);
  p.play(BLUE);
  EXPECT_EQ(p.played(BLUE), 1);
  p.play(Card(BLUE, 2));
  EXPECT_EQ(p.played(BLUE), 2);
}

TEST_F(GameTest, played_perm) {
  Played p{0, 1, 3, 2, 4};
  Perm perm{3, 2, 4, 1, 0};
  p.permute(perm);
  Played ex{4, 2, 1, 0, 3};
  EXPECT_EQ(p, ex);

  EXPECT_TRUE(p.getPlayables().has(Card(0, 5)));
  EXPECT_TRUE(p.getPlayables().has(Card(3, 1)));
  EXPECT_TRUE(p.getDiscardables().has(Card(0, 4)));
  EXPECT_FALSE(p.getDiscardables().has(Card(2, 3)));
}

////////////////////////////////////////////////////////////////////////////////
/// Discarded
////////////////////////////////////////////////////////////////////////////////

TEST_F(GameTest, discard_ctor) {
  Discarded d;
  EXPECT_EQ(d.numDiscarded(), 0);
  EXPECT_EQ(d.numDiscarded(Card(BLUE, 2)), 0);
}

TEST_F(GameTest, discard_nums) {
  Discarded d;
  d.discard(Card(BLUE, 2), 5);
  d.discard(Card(YELLOW, 1));
  EXPECT_EQ(d.numDiscarded(), 6);
  EXPECT_EQ(d.numDiscarded(Card(BLUE, 2)), 5);
  EXPECT_EQ(d.numDiscarded(Card(YELLOW, 1)), 1);
}

TEST_F(GameTest, discard_perm) {
  Discarded d;
  d.discard(Card(BLUE, 2), 2);
  d.discard(Card(RED, 5));
  Perm p{3, 4, 0, 1, 2};
  d.permute(p);
  EXPECT_EQ(d.numDiscarded(), 3);
  EXPECT_EQ(d.numDiscarded(Card(WHITE, 2)), 2);
  EXPECT_EQ(d.numDiscarded(Card(BLUE, 5)), 1);
}

////////////////////////////////////////////////////////////////////////////////
/// Game
////////////////////////////////////////////////////////////////////////////////

TEST_F(GameTest, game_adv) {
  GameState g(5);
  EXPECT_EQ(g.currentPlayer, 0);
  g.advanceTurn();
  EXPECT_EQ(g.currentPlayer, 1);
  g.advanceTurn();
  g.advanceTurn();
  g.advanceTurn();
  EXPECT_EQ(g.currentPlayer, 4);
  g.advanceTurn();
  EXPECT_EQ(g.currentPlayer, 0);

  EXPECT_EQ(g.turn, 5);
}

TEST_F(GameTest, game_dropndraw) {
  GameState g(5);

  g.hands[1] = htest;

  g.dropAndDraw(1, 1, Card(4, 5));
  EXPECT_EQ(g.hands[1].size(), 4);
  EXPECT_EQ(g.hands[1][0], Card(0, 1));
  EXPECT_EQ(g.hands[1][1], Card(1, 1));
  EXPECT_EQ(g.hands[1][3], Card(4, 5));

  g.dropAndDraw(1, 1, Card());
  g.dropAndDraw(1, 1, Card());
  EXPECT_EQ(g.hands[1].size(), 2);
}

TEST_F(GameTest, basicAction) {
  GameState g(5);

  g.hands[0] = htest;

  auto a1 = g.resolve(2, Decision::hint_value(0, 1));
  EXPECT_EQ(a1.actor, 2);
  ASSERT_EQ(a1.type, HINT_VALUE);
  std::array<bool, 5> expected = {1, 0, 1, 0, 0};
  EXPECT_EQ(a1.positions, expected);

  auto a2 = g.resolve(0, Decision::play(1));
  EXPECT_EQ(a2.actor, 0);
  ASSERT_EQ(a2.type, PLAY);
  EXPECT_EQ(a2.card, Card(0, 3));
}

TEST_F(GameTest, basicUpdate) {
  NaturesGameState g(5, 0);

  g.hands[0] = htest;
  Action a1{0, PLAY, 2, Card(3, 1)};
  g.update(a1);
  EXPECT_EQ(g.played(3), 1);
  EXPECT_EQ(g.hands[0].size(), 4);
  EXPECT_EQ(g.actionsLeft, -1);
  g.actionsLeft = 4;
  Action a2{0, PLAY, 0, Card(3, 2)};
  g.update(a2);
  EXPECT_EQ(g.played(3), 2);
  EXPECT_EQ(g.actionsLeft, 3);
}

TEST_F(GameTest, game_view) {
  NaturesGameState g(5, 0);

  GameView v(&g, 2);
  EXPECT_EQ(v->score(), 0);
  EXPECT_EQ(v[1].getIdx(), g.hands[3].getIdx());
  EXPECT_EQ(v[2].getIdx(), g.hands[4].getIdx());
  EXPECT_EQ(v[3].getIdx(), g.hands[0].getIdx());
  EXPECT_EQ(v[4].getIdx(), g.hands[1].getIdx());
}
