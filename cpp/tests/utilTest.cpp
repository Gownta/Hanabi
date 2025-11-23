#include "experimental/njormrod/hanabi/util.h"
#include <gtest/gtest.h>

class UtilTest : public ::testing::Test {
 public:
  UtilTest() : g(4) {}

  void SetUp() override {
    initCards(false);

    g.play(BLUE);
    g.play(BLUE);
    g.play(RED);
    g.play(RED);
    g.play(WHITE);
    g.play(WHITE);
    g.play(WHITE);
    g.play(WHITE);
    g.play(WHITE);
    g.play(YELLOW);
  }

  PublicGameState g;
};

TEST_F(UtilTest, playable) {
  std::set<Card> playables;
  playables.insert(Card(0, 3));
  playables.insert(Card(1, 1));
  playables.insert(Card(2, 3));
  playables.insert(Card(4, 2));

  for (auto c : uniqueCards()) {
    if (playables.count(c)) {
      EXPECT_TRUE(g.isPlayable(c));
    } else {
      EXPECT_FALSE(g.isPlayable(c));
    }
    EXPECT_EQ(g.isPlayable(c), g.isPlayable(c));
  }
}

TEST_F(UtilTest, discardable) {
  std::set<Card> discardables;
  discardables.insert(Card(0, 1));
  discardables.insert(Card(0, 2));
  discardables.insert(Card(2, 1));
  discardables.insert(Card(2, 2));
  discardables.insert(Card(3, 1));
  discardables.insert(Card(3, 2));
  discardables.insert(Card(3, 3));
  discardables.insert(Card(3, 4));
  discardables.insert(Card(3, 5));
  discardables.insert(Card(4, 1));

  for (auto c : uniqueCards()) {
    if (discardables.count(c)) {
      EXPECT_TRUE(g.isDiscardable(c));
    } else {
      EXPECT_FALSE(g.isDiscardable(c));
    }
    EXPECT_EQ(g.isDiscardable(c), g.isDiscardable(c));
  }
}

TEST_F(UtilTest, num_playable) {
  // 2 0 2 5 1
  Hand h;
  EXPECT_EQ(numPlayable(h, g), 0);
  h.add(BLUE, 3);
  EXPECT_EQ(numPlayable(h, g), 1);
  h.add(BLUE, 3);
  EXPECT_EQ(numPlayable(h, g), 2);
  h.add(BLUE, 4);
  EXPECT_EQ(numPlayable(h, g), 2);
  h.add(GREEN, 1);
  EXPECT_EQ(numPlayable(h, g), 3);
}

/*
TEST_F(UtilTest, crits) {
  std::array<Value, 5> played;
  played.fill(0);
  std::array<int8_t, 26> discarded;
  discarded.fill(0);

  // normal
  played[BLUE] = 0;
  discarded[Card(BLUE, 3).getIdx()] = 1;

  // post-play
  played[GREEN] = 3;
  discarded[Card(GREEN, 3).getIdx()] = 1;

  // unreachable
  played[RED] = 0;
  discarded[Card(RED, 4).getIdx()] = 2;

  // 5 played
  played[WHITE] = 5;

  auto c5bu = analyzeCriticals(played, discarded);
  EXPECT_EQ(c5bu[0], 1);
  EXPECT_EQ(c5bu[1], 3);
  EXPECT_EQ(c5bu[2], 2);
  EXPECT_EQ(c5bu[3], 2);
}*/

TEST_F(UtilTest, permutation) {
  auto [id, perm, inv] = getPermutation(g);

  EXPECT_FALSE(id);
  Perm eperm{2, 0, 3, 4, 1};
  Perm einv{1, 4, 0, 2, 3};

  EXPECT_EQ(perm, eperm);
  EXPECT_EQ(inv, einv);

  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(inv[perm[i]], i);
  }
}

TEST_F(UtilTest, perm_card) {
  Perm perm{0, 4, 3, 2, 1};
  Card c(2, 3);
  Card pc(3, 3);
  EXPECT_EQ(permute(perm, c), pc);
}

TEST_F(UtilTest, perm_hand) {
  Perm perm{0, 4, 3, 2, 1};
  Hand h;
  Hand ph;
  h.add(Card(2, 3));
  h.add(Card(3, 4));
  ph.add(Card(3, 3));
  ph.add(Card(2, 4));
  EXPECT_EQ(permute(perm, h), ph);
}
