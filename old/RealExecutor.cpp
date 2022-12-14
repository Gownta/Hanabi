#include "Executor.h"
#include "HPlayer.h"

class RealExecutor : public Executor {
 public:
  RealExecutor(int numPlayers, unsigned seed) : Executor(numPlayers, seed) {
    players.reserve(numPlayers);
    for (int i = 0; i < numPlayers; ++i) {
      GameView v(&g_, i);
      players.emplace_back(v);
    }
  }

  Decision act(const NaturesGameState& g) override {
    return players[g.currentPlayer].act();
  }

  void inform(const Action& action) override {
    auto cp = action;
    for (int i = 0; i < players.size(); ++i) {
      cp.actor = action.actor - i;
      if (cp.actor < 0) {
        cp.actor += players.size();
      }
      players[i].inform(cp);
    }
  }

  std::vector<HPlayer> players;
};

std::unique_ptr<Executor> makeRealExecutor(int numPlayers, unsigned seed) {
  return std::make_unique<RealExecutor>(numPlayers, seed);
}
