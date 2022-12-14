#include "GameState.h"
#include <algorithm>
#include <cassert>
#include <numeric>

////////////////////////////////////////////////////////////////////////////////
/// Played
////////////////////////////////////////////////////////////////////////////////

Played::Played() : playable_(1) {
  played_.fill(0);
}

Played::Played(std::initializer_list<Value> l) {
  assert(l.size() == 5);
  std::copy(l.begin(), l.end(), played_.begin());
  for (int i = 0; i < 5; ++i) {
    if (played_[i] != 5) {
      playable_.add(Card(i, played_[i] + 1));
    }
    for (int j = 1; j <= played_[i]; ++j) {
      discardable_.add(Card(i, j));
    }
  }
}

Value Played::played(Colour c) const {
  return played_[c];
}

Value Played::played(Card c) const {
  return played(c.colour());
}

bool Played::isPlayable(Card c) const {
  return played_[c.colour()] + 1 == c.value();
}

bool Played::isDiscardable(Card c) const {
  return played_[c.colour()] >= c.value();
}

CardSet Played::getPlayables() const {
  return playable_;
}

CardSet Played::getDiscardables() const {
  return discardable_;
}

void Played::play(Colour c) {
  played_[c]++;
  discardable_.add(Card(c, played_[c]));
  playable_.remove(Card(c, played_[c]));
  if (played_[c] != 5) {
    playable_.add(Card(c, played_[c] + 1));
  }
}

void Played::play(Card c) {
  play(c.colour());
}

int Played::numPlayed() const {
  return std::accumulate(played_.begin(), played_.end(), 0);
}

bool Played::operator==(const Played& other) const {
  return played_ == other.played_;
}

////////////////////////////////////////////////////////////////////////////////
/// Discarded
////////////////////////////////////////////////////////////////////////////////

Discarded::Discarded() : total_(0) {
  discarded_.fill(0);
}

int8_t Discarded::numDiscarded() const {
  return total_;
}
int8_t Discarded::numDiscarded(Card c) const {
  return discarded_[c.getIdx()];
}

void Discarded::discard(Card c, int count) {
  discarded_[c.getIdx()] += count;
  total_ += count;
}

////////////////////////////////////////////////////////////////////////////////
/// PublicGameState
////////////////////////////////////////////////////////////////////////////////

PublicGameState::PublicGameState(int np) : numPlayers(np) {}

void PublicGameState::playCard(Card card) {
  if (isPlayable(card)) {
    play(card);
    if (card.value() == 5 && numHints < 8) {
      numHints++;
    }
  } else {
    discard(card);
    numBombsLeft--;
  }
}

void PublicGameState::discardCard(Card card) {
  assert(numHints < 8);
  discard(card);
  numHints++;
}

bool PublicGameState::isOver() const {
  if (numBombsLeft == 0) {
    return true;
  }
  if (actionsLeft == 0) {
    return true;
  }
  for (int i = 0; i < 5; ++i) {
    if (played(i) != 5) {
      return false;
    }
  }
  return true;
}

int PublicGameState::score() const {
  if (numBombsLeft == 0) {
    return 0;
  }
  return numPlayed();
}

GameState::GameState(int np) : PublicGameState(np) {
  hands.fill(Hand());
}

Action GameState::resolve(int player, const Decision& decision) const {
  using R = std::
      variant<PlayAction, DiscardAction, ColourHintAction, ValueHintAction>;
  return Action{
      player,
      std::visit(
          overloaded{[&, this](const PlayDecision& d) -> R {
                       assert(0 <= d.pos && d.pos < hands[player].size());
                       return PlayAction{d.pos, hands[player][d.pos]};
                     },
                     [&, this](const DiscardDecision& d) -> R {
                       assert(0 <= d.pos && d.pos < hands[player].size());
                       assert(numHints != 8);
                       return DiscardAction{d.pos, hands[player][d.pos]};
                     },
                     [&, this](const ColourHintDecision& d) -> R {
                       assert(numHints != 0);
                       std::array<bool, 5> positions;
                       positions.fill(false);
                       bool any = false;
                       for (int i = 0; i < hands[d.player].size(); ++i) {
                         positions[i] = hands[d.player][i].colour() == d.colour;
                         if (positions[i]) {
                           any = true;
                         }
                       }
                       assert(any);
                       return ColourHintAction{d.player, d.colour, positions};
                     },
                     [&, this](const ValueHintDecision& d) -> R {
                       assert(numHints != 0);
                       std::array<bool, 5> positions;
                       positions.fill(false);
                       bool any = false;
                       for (int i = 0; i < hands[d.player].size(); ++i) {
                         positions[i] = hands[d.player][i].value() == d.value;
                         if (positions[i]) {
                           any = true;
                         }
                       }
                       assert(any);
                       return ValueHintAction{d.player, d.value, positions};
                     }},
          decision)};
}

void GameState::dropAndDraw(int player, int pos, Card draw) {
  hands[player].erase(pos);
  if (draw != Card()) {
    hands[player].add(draw);
  }
}

void GameState::advanceTurn() {
  currentPlayer++;
  if (currentPlayer == numPlayers) {
    currentPlayer = 0;
  }
  turn++;
}

NaturesGameState::NaturesGameState(int np, unsigned s)
    : GameState(np), deck(s), seed(s) {
  int cardsPerPlayer = np > 3 ? 4 : 5;
  for (int i = 0; i < numPlayers; ++i) {
    for (int j = 0; j < cardsPerPlayer; ++j) {
      hands[i].add(deck.draw());
      deckSize--;
    }
  }
}

void NaturesGameState::update(const Action& action) {
  assert(actionsLeft != 0);
  if (actionsLeft > 0) {
    actionsLeft--;
  }
  auto toDraw = [&]() {
    if (deck.empty()) {
      return Card();
    }
    Card ret = deck.draw();
    deckSize--;
    if (deck.empty()) {
      actionsLeft = numPlayers;
    }
    return ret;
  };
  std::visit(
      overloaded{[&, this](const PlayAction& a) {
                   playCard(a.card);
                   dropAndDraw(action.actor, a.pos, toDraw());
                 },
                 [&, this](const DiscardAction& a) {
                   discardCard(a.card);
                   dropAndDraw(action.actor, a.pos, toDraw());
                 },
                 [&, this](auto&) { numHints--; }},
      action.what);
}

////////////////////////////////////////////////////////////////////////////////
/// GameView
////////////////////////////////////////////////////////////////////////////////

GameView::GameView(const GameState* gs, int player) : g(gs), myPos(player) {}

const PublicGameState* GameView::operator->() const {
  return g;
}

// 0 is self, which is disallowed
Hand GameView::operator[](int pos) const {
  assert(pos != 0);
  int actual = pos + myPos;
  if (actual >= g->numPlayers) {
    actual -= g->numPlayers;
  }
  return g->hands[actual];
}
