#pragma once

#include <memory>
#include <vector>
#include "Action.h"
#include "GameState.h"

class Executor {
 public:
  Executor(int numPlayers, unsigned seed);
  virtual ~Executor() = default;

  // return the score
  int play(bool interactive);

  virtual Decision act(const NaturesGameState& g) = 0;
  virtual void inform(const Action& action) {}

 protected:
  NaturesGameState g_;
  std::vector<Action> actions_;
};

std::unique_ptr<Executor> makeExecutor(int numPlayers, unsigned seed);
std::unique_ptr<Executor> makeSeerExecutor(int numPlayers, unsigned seed);
std::unique_ptr<Executor> makeRealExecutor(int numPlayers, unsigned seed);
