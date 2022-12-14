#include "Executor.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include "Printer.h"

Executor::Executor(int numPlayers, unsigned seed) : g_(numPlayers, seed) {}

int Executor::play(bool interactive) {
  for (;;) {
    if (interactive) {
      printGame(g_, &actions_[0], &actions_[0] + actions_.size());
    }
    auto decision = act(g_);
    if (interactive) {
      std::cout << "Press any key to continue...\n";
      std::cin.get(); // press a key to continue
    }
    auto action = g_.resolve(g_.currentPlayer, decision);
    actions_.push_back(action);
    g_.update(action);
    if (g_.isOver()) {
      if (interactive) {
        printGame(g_, &actions_[0], &actions_[0] + actions_.size());
      }
      return g_.score();
    }
    inform(action);
    g_.advanceTurn();
  }
}

std::unique_ptr<Executor> makeExecutor(int numPlayers, unsigned seed) {
  return makeSeerExecutor(numPlayers, seed);
}
