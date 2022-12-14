#pragma once

#include <array>
#include <chrono>
#include <variant>
#include "Cards.h"

// Decisions are what players decide to do
// Actions are what ends up happening

struct PlayDecision {
  int pos;
};

struct DiscardDecision {
  int pos;
};

struct ColourHintDecision {
  int player;
  Colour colour;
};

struct ValueHintDecision {
  int player;
  Value value;
};

using Decision = std::variant<
    PlayDecision,
    DiscardDecision,
    ColourHintDecision,
    ValueHintDecision>;

struct PlayAction {
  int actor;
  int pos;
  Card card;
};

struct DiscardAction {
  int actor;
  int pos;
  Card card;
};

struct ColourHintAction {
  int actor;
  int hintee;
  Colour colour;
  std::array<bool, 4> positions;
};

struct ValueHintAction {
  int actor;
  int hintee;
  Value value;
  std::array<bool, 4> positions;
};

using Action =
    std::variant<PlayAction, DiscardAction, ColourHintAction, ValueHintAction>;

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;
