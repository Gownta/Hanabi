#include "experimental/njormrod/hanabi/Cards.h"
#include <gtest/gtest.h>
#include <set>

class CardsTest : public ::testing::Test {
 public:
  void SetUp() override {
    initCards(false);
  }
};

TEST_F(CardsTest, ncpv) {
  const auto& ncpv = numCardsPerValue();
  EXPECT_EQ(ncpv[1], 3);
  EXPECT_EQ(ncpv[2], 2);
  EXPECT_EQ(ncpv[3], 2);
  EXPECT_EQ(ncpv[4], 2);
  EXPECT_EQ(ncpv[5], 1);
}

TEST_F(CardsTest, card) {
  Card c;
  Card b4(0, 4);

  EXPECT_EQ(c.getIdx(), 0);
  EXPECT_NE(b4.getIdx(), 0);
  EXPECT_LT(b4.getIdx(), 26);

  EXPECT_EQ(b4.colour(), 0);
  EXPECT_EQ(b4.value(), 4);

  EXPECT_EQ(c, c);
  EXPECT_EQ(b4, b4);
  EXPECT_NE(c, b4);
}

TEST_F(CardsTest, datarays) {
  std::set<int> indices;
  for (auto c : uniqueCards()) {
    auto idx = c.getIdx();
    EXPECT_LT(0, idx);
    EXPECT_LT(idx, 26);
    indices.insert(idx);
  }
  EXPECT_EQ(indices.size(), 25);
}

TEST_F(CardsTest, cardset) {
  CardSet cs;
  Card c(0, 2);
  Card d(1, 4);
  EXPECT_FALSE(cs.has(c));
  cs.add(c);
  EXPECT_TRUE(cs.has(c));
  cs.add(c);
  cs.add(d);
  EXPECT_TRUE(cs.has(c));
  cs.remove(c);
  EXPECT_FALSE(cs.has(c));
  EXPECT_TRUE(cs.has(d));
}

TEST_F(CardsTest, cardset5) {
  CardSet c5(5);
  Card b2(0, 2);
  Card b5(0, 5);
  Card y5(4, 5);
  EXPECT_FALSE(c5.has(b2));
  EXPECT_TRUE(c5.has(b5));
  EXPECT_TRUE(c5.has(y5));
}

TEST_F(CardsTest, perm) {
  CardSet cs;
  cs.add(Card(GREEN, 3));
  cs.add(Card(BLUE, 3));
  cs.add(Card(RED, 1));

  Perm p{1, 0, 4, 3, 2};
  cs.permute(p);
  EXPECT_TRUE(cs.has(Card(BLUE, 3)));
  EXPECT_TRUE(cs.has(Card(GREEN, 3)));
  EXPECT_TRUE(cs.has(Card(YELLOW, 1)));
  EXPECT_FALSE(cs.has(Card(RED, 1)));
}

TEST_F(CardsTest, hand) {
  Hand h;
  std::vector<Card> v;

  auto e = [&](const char* msg = "") {
    bool eq = std::equal(v.begin(), v.end(), h.begin(), h.end());
    ASSERT_TRUE(eq) << msg;

    auto b = h.begin();
    auto e = h.end();
    ASSERT_EQ(h.size(), e - b) << msg;
    for (int i = 0; i < h.size(); ++i, ++b) {
      EXPECT_EQ(*b, h[i]) << msg;
    }

    auto vs = v;
    std::sort(vs.begin(), vs.end());
    auto hs = h.sorted();
    bool seq = std::equal(v.begin(), v.end(), h.begin(), h.end());
    EXPECT_TRUE(seq) << msg;
  };

  Card b1(0, 1), b2(0, 2), g3(1, 3), y5(4, 5);
  EXPECT_EQ(h.getIdx(), 0);
  e("init");

  h.add(b1);
  v.push_back(b1);
  e("b1");

  h.add(b2);
  v.push_back(b2);
  e("b1b2");

  h.add(b1);
  v.push_back(b1);
  e("b1b2b1");

  h.add(y5);
  v.push_back(y5);
  e("b1b2b1y5");

  h.erase(1);
  v.erase(v.begin() + 1);
  e("b1b1y5");

  h.erase(2);
  v.erase(v.begin() + 2);
  e("b1b1");

  h.add(g3);
  v.push_back(g3);
  e("b1b1g3");

  h.erase(0);
  v.erase(v.begin() + 0);
  e("b1g3");

  h.erase(0);
  v.erase(v.begin() + 0);
  e("g3");

  h.erase(0);
  v.erase(v.begin() + 0);
  e("end");
}

TEST_F(CardsTest, handCardSet) {
  Hand h;
  CardSet cs;

  Card b1(BLUE, 1);
  Card g2(GREEN, 2);

  h.add(b1);
  h.add(b1);
  cs.add(b1);

  h.add(g2);
  cs.add(g2);

  EXPECT_EQ(h.getCardSet().getIdx(), cs.getIdx());
}

TEST_F(CardsTest, addCV) {
  Hand h1, h2;
  h1.add(Card(RED, 2));
  h2.add(RED, 2);
  EXPECT_EQ(h1, h2);
}

TEST_F(CardsTest, allHands) {
  auto hs = allHands(1);
  ASSERT_EQ(hs.size(), 25);
  std::set<Card> cards;
  for (auto h : hs) {
    ASSERT_EQ(h.size(), 1);
    cards.insert(h[0]);
  }
  EXPECT_EQ(cards.size(), 25);
}

TEST_F(CardsTest, deck) {
  Deck d1(0), d2(0);
  std::array<Card, 50> cards;
  for (int i = 0; i < 50; ++i) {
    ASSERT_EQ(50 - i, d1.size());
    cards[i] = d1.draw();
    EXPECT_EQ(d2.draw(), cards[i]);
  }
  EXPECT_TRUE(d1.empty());
  std::sort(cards.begin(), cards.end());
  auto all = allCards();
  std::sort(all.begin(), all.end());
  EXPECT_TRUE(std::equal(cards.begin(), cards.end(), all.begin(), all.end()));
}
