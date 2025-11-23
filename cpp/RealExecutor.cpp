#include "Executor.h"
#include "IPlayer.h"

class RealExecutor : public Executor {
 public:
  RealExecutor(int numPlayers, unsigned seed) : Executor(numPlayers, seed) {
    players.reserve(numPlayers);
    for (int i = 0; i < numPlayers; ++i) {
      GameView v(&g_, i);
      players.emplace_back(makeXPlayer(v));
    }
  }

  Decision act(const NaturesGameState& g) override {
    auto ret = players[g.currentPlayer]->act();
    if (ret.type == HINT_COLOUR || ret.type == HINT_VALUE) {
      ret.hintee += g.currentPlayer;
      if (ret.hintee >= players.size()) {
        ret.hintee -= players.size();
      }
    }
    return ret;
  }

  void inform(const Action& action) override {
    auto cp = action;
    for (int i = 0; i < players.size(); ++i) {
      cp.actor = action.actor - i;
      if (cp.actor < 0) {
        cp.actor += players.size();
      }
      players[i]->inform(cp);
    }
  }

  std::vector<std::unique_ptr<IPlayer>> players;
};

std::unique_ptr<Executor> makeRealExecutor(int numPlayers, unsigned seed) {
  return std::make_unique<RealExecutor>(numPlayers, seed);
}
