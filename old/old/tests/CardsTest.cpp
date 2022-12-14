#include "experimental/njormrod/hanabi/Cards.h"
#include <gtest/gtest.h>
#include <set>

TEST(cards, index) {
  std::set<int> seen;
  for (auto card : uniqueCards()) {
    auto idx = card.index();
    EXPECT_LE(0, idx);
    EXPECT_LT(idx, NUM_UNIQUE_CARDS);
    EXPECT_FALSE(seen.count(idx));
    seen.insert(idx);
  }
}
