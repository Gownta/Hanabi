#include "ModularEngine.h"
#include "common/logging/logging.h"
#include "experimental/njormrod/hanabi/Player.h"

bool has(const Hand& hand, Colour c) {
  for (const auto& card : hand) {
    if (card.colour == c) {
      return true;
    }
  }
  return false;
}

class ModuloPlayer : public Player {
 public:
  ModuloPlayer(View v) : Player(v), modEngine_(v, true, true) {}

  Decision act() override {
    // Is there a modular hint I must attend to?
    // Am I in sequence?
    // Can I hint?
    //  - If low on hints, reconsider
    //  - If leads to disaster, reconsider
    // Discard

    // Option #1: obey checksum
    auto cksmPlay = modEngine_.toPlay();
    if (cksmPlay && (*cksmPlay)[0] != 4) {
      return PlayDecision{(*cksmPlay)[0]};
    }

    // Option #2: give checksum hint
    auto [cksm, positions] = modEngine_.computeModular(3);
    // TODO what about when player 2 is going to play the Red4, but
    // player 1 sees that player 3 has Red4, so re-hints. -_-
    int numNewPlayables = countExtraPlays(cksmPlay, positions);
    if (view.getNumHints() && numNewPlayables > 0) {
      Colour tohint = (Colour)(cksm % 5);
      for (int i = 1; i <= 3; ++i) {
        int who = (view.getMyPos() + i) % 4;
        if (has(view.getHand(who), tohint)) {
          return ColourHintDecision{who, tohint};
        }
      }
      // oh no! We cannot hint that colour
      // TODO
    }

    // Option #3: discard
    if (view.getNumHints() == STARTING_HINTS) {
      int who = (view.getMyPos() + 1) % 4;
      return ValueHintDecision{who, view.getHand(who)[0].value};
    }
    return DiscardDecision{0};
  }

  void update(const Action& action) override {
    modEngine_.update(action);
  }

 private:
  ModularEngine modEngine_;
};

std::unique_ptr<Player> createModuloPlayer(View v) {
  return std::make_unique<ModuloPlayer>(v);
}
