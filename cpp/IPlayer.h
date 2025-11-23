#pragma once

#include <memory>
#include "Action.h"
#include "GameState.h"

class IPlayer {
 public:
  IPlayer(GameView v);
  virtual ~IPlayer() = default;

  /**
   * Decisions and actions are rotated so that the player sees themselves as
   * player 0.
   */
  virtual Decision act() = 0;
  virtual void inform(const Action& action) = 0;

 protected:
  GameView v;
};

std::unique_ptr<IPlayer> makeXPlayer(GameView v);
