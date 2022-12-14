#pragma once

#include "Action.h"
#include "GameState.h"

class Player {
 public:
  Player(const GameState& g, int p, Token tok) : game(g), pos(p), token(tok) {}
  virtual ~Player() = default;

  virtual Decision act() = 0;

  // Called once the action has been executed on the board,
  // but the turn has not yet been advanced
  virtual void update(const Action& action) {}

  const GameState& game;
  int pos;
  Token token;
};

// std::unique_ptr<Player> createModuloPlayer(View view);
std::unique_ptr<Player> createSeer(const GameState& game, int pos, Token omni);
