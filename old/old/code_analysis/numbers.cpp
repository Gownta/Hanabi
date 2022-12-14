#include <iostream>
#include <map>
#include <set>
#include "experimental/njormrod/hanabi/GameState.h"
using namespace std;

void n0() {
  int i[4];
  set<int> reprs;
  auto f = [](int idx) {
    if (idx % 3 == 0)
      return idx;
    return 0;
  };
  for (i[0] = 0; i[0] < allCards().size(); ++i[0]) {
    for (i[1] = i[0] + 1; i[1] < allCards().size(); ++i[1]) {
      for (i[2] = i[1] + 1; i[2] < allCards().size(); ++i[2]) {
        for (i[3] = i[2] + 1; i[3] < allCards().size(); ++i[3]) {
          int32_t repr = 15625 * f(allCards()[i[0]].index()) +
              625 * f(allCards()[i[1]].index()) +
              25 * f(allCards()[i[2]].index()) + 1 * (allCards()[i[3]].index());
          reprs.insert(repr);
        }
      }
    }
  }
  cout << "Number of ways where only 70%% of cards count: " << reprs.size()
       << endl;
}
void n1() {
  int i[4];
  set<int> reprs;
  for (i[0] = 0; i[0] < allCards().size(); ++i[0]) {
    for (i[1] = i[0] + 1; i[1] < allCards().size(); ++i[1]) {
      for (i[2] = i[1] + 1; i[2] < allCards().size(); ++i[2]) {
        for (i[3] = i[2] + 1; i[3] < allCards().size(); ++i[3]) {
          int32_t repr = 15625 * allCards()[i[0]].index() +
              625 * allCards()[i[1]].index() + 25 * allCards()[i[2]].index() +
              1 * allCards()[i[3]].index();
          reprs.insert(repr);
        }
      }
    }
  }
  cout << "Number of distinct hands = " << reprs.size() << endl;
}
void n1unsorted() {
  int i[4];
  set<int> reprs;
  for (i[0] = 0; i[0] < allCards().size(); ++i[0]) {
    for (i[1] = 0; i[1] < allCards().size(); ++i[1]) {
      for (i[2] = 0; i[2] < allCards().size(); ++i[2]) {
        for (i[3] = 0; i[3] < allCards().size(); ++i[3]) {
          int j[4] = {i[0], i[1], i[2], i[3]};
          std::sort(j, j + 4);
          if (std::adjacent_find(j, j + 4) != j + 4) {
            continue;
          }
          int32_t repr = 15625 * allCards()[i[0]].index() +
              625 * allCards()[i[1]].index() + 25 * allCards()[i[2]].index() +
              1 * allCards()[i[3]].index();
          reprs.insert(repr);
        }
      }
    }
  }
  cout << "Number of distinct unsorted hands = " << reprs.size() << endl;
}

void n2() {
  map<int, int> ways;
  int i[5];
  for (i[0] = 0; i[0] <= 5; ++i[0]) {
    for (i[1] = i[0]; i[1] <= 5; ++i[1]) {
      for (i[2] = i[1]; i[2] <= 5; ++i[2]) {
        for (i[3] = i[2]; i[3] <= 5; ++i[3]) {
          for (i[4] = i[3]; i[4] <= 5; ++i[4]) {
            int s = i[0] + i[1] + i[2] + i[3] + i[4];
            ways[s]++;
          }
        }
      }
    }
  }
  cout << "Number of ways to have an increasing table with a given score:\n";
  for (auto& [score, many] : ways) {
    cout << score << " ways: " << many << endl;
  }
}

int main() {
  cout << "O hi\n";
  n0();
  n1();
  n1unsorted();
  n2();
}
