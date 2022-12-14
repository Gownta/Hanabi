#include "Executor.h"
#include <glog/logging.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "GameState.h"
#include "Printer.h"

int execute(
    int numPlayers,
    std::function<std::unique_ptr<Player>(const GameState&, int)> playerBuilder,
    unsigned seed,
    bool interactive) {
  assert(2 <= numPlayers && numPlayers <= 5);
  GameState game(numPlayers, seed);

  std::vector<std::unique_ptr<Player>> players;
  for (int i = 0; i < numPlayers; ++i) {
    players.push_back(playerBuilder(game, i));
  }
  Token printTok(numPlayers, seed);

  for (;;) {
    if (interactive) {
      printGame(game, printTok);
    }
    int pos = game.currentPlayer;
    auto decision = players[pos]->act();
    if (interactive) {
      std::cout << "Press any key to continue...\n";
      std::cin.get(); // press a key to continue
    }
    auto action = game.resolve(pos, decision);
    game.update(action);
    if (game.isOver()) {
      break;
    }
    for (auto& p : players) {
      p->update(action);
    }
    game.advanceTurn();
  }
  if (interactive) {
    printGame(game, printTok);
  }
  assert(game.numBombsLeft == 3);
  return game.score;
}
