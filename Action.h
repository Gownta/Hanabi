#pragma once

#include <array>
#include <chrono>
#include <variant>
#include "Cards.h"

// Decisions are what players decide to do
// Actions are what ends up happening

enum ActionType {
  PLAY,
  HINT_COLOUR,
  HINT_VALUE,
  DISCARD,
};

struct Decision {
  static Decision play(int pos);
  static Decision discard(int pos);
  static Decision hint_colour(int hintee, Colour colour);
  static Decision hint_value(int hintee, Value value);

  ActionType type;

  // For plays and discards
  int pos;

  // For hints
  int hintee;
  Colour colour;
  Value value;
};

struct Action {
  int actor;
  ActionType type;

  // For plays and discards
  int pos;
  Card card;

  // For hints
  int hintee;
  Colour colour;
  Value value;
  std::array<bool, 5> positions;
};
