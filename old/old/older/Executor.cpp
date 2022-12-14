#include "Executor.h"
#include <glog/logging.h>
#include <iostream>
#include <map>
#include <vector>
#include "Brute.h"
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

namespace {

// deck size
// last turn
// current player
// ** hands
// played
// discarded
// hints
// bombs
using GameRepr = std::vector<int32_t>;
GameRepr serialize(const GameState& game, const Token& tok) {
  GameRepr repr;
  repr.push_back(game.deck.size());
  repr.push_back(game.lastTurn);
  repr.push_back(game.currentPlayer);
  for (int i = 0; i < NUM_COLOURS; ++i) {
    repr.push_back(game.played[i]);
  }
  for (int i = 0; i < NUM_UNIQUE_CARDS; ++i) {
    repr.push_back(game.discarded[i]);
  }
  repr.push_back(game.numHints);
  repr.push_back(game.numBombsLeft);

  for (int i = 0; i < game.numPlayers; ++i) {
    const auto& hand = game.getHand(tok, i);
    for (const auto& card : hand) {
      repr.push_back(card.colour);
      repr.push_back(card.value);
    }
  }

  return repr;
}

int go(
    const GameState& game,
    const Token& tok,
    const std::vector<Brute>& brutes,
    int& achievable,
    std::map<GameRepr, int>& cachedScores,
    int depth) {
  if (game.isOver()) {
    achievable = std::max(achievable, game.score);
    return game.score;
  }
  if (game.maxScore <= achievable) {
    return achievable;
  }

  auto repr = serialize(game, tok);
  auto cit = cachedScores.find(repr);
  if (cit != cachedScores.end()) {
    return cit->second;
  }

  int pos = game.currentPlayer;
  auto decisions = brutes[pos].actions(game);
  int maxScore = 0;
  int i = 0;
  for (auto& d : decisions) {
    ++i;
    LOG_EVERY_N(INFO, 10000) << "Processing at depth " << depth << ": " << i
                             << "/" << decisions.size();
    GameState g2(game);
    auto action = g2.resolve(pos, d);
    g2.update(action);
    g2.advanceTurn();
    int score =
        go(std::move(g2), tok, brutes, achievable, cachedScores, depth + 1);
    maxScore = std::max(maxScore, score);
  }

  cachedScores[repr] = maxScore;
  return maxScore;
} // namespace

} // namespace

int brute(int numPlayers, unsigned seed) {
  assert(2 <= numPlayers && numPlayers <= 5);
  GameState game(numPlayers, seed);
  std::vector<Brute> brutes;
  Token omni(numPlayers, seed);
  for (int i = 0; i < numPlayers; ++i) {
    brutes.emplace_back(i, omni);
  }

  std::map<GameRepr, int> cachedScores;
  int achievableScore = 0;
  return go(game, omni, brutes, achievableScore, cachedScores, 0);
}
