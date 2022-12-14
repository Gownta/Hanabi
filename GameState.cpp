#include "GameState.h"
#include <algorithm>
#include <cassert>
#include <numeric>

#include "common/logging/logging.h"

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

void Played::permute(const Perm& p) {
  auto cp = played_;
  for (int i = 0; i < 5; ++i) {
    played_[p[i]] = cp[i];
  }
  playable_.permute(p);
  discardable_.permute(p);
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

void Discarded::permute(const Perm& p) {
  auto cp = discarded_;
  for (auto c : uniqueCards()) {
    discarded_[Card(p[c.colour()], c.value()).getIdx()] = cp[c.getIdx()];
  }
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

void PublicGameState::permute(const Perm& p) {
  Played::permute(p);
  Discarded::permute(p);
}

GameState::GameState(int np) : PublicGameState(np) {
  hands.fill(Hand());
}

Action GameState::resolve(int player, const Decision& decision) const {
  Action ret;
  ret.actor = player;
  ret.type = decision.type;

  switch (decision.type) {
    case PLAY: {
      assert(0 <= decision.pos && decision.pos < hands[player].size());
      ret.pos = decision.pos;
      ret.card = hands[player][decision.pos];
      return ret;
    }
    case DISCARD: {
      assert(0 <= decision.pos && decision.pos < hands[player].size());
      assert(numHints != 8);
      ret.pos = decision.pos;
      ret.card = hands[player][decision.pos];
      return ret;
    }
    case HINT_COLOUR: {
      assert(numHints != 0);
      ret.hintee = decision.hintee;
      ret.colour = decision.colour;
      ret.positions.fill(false);
      bool any = false;
      for (int i = 0; i < hands[decision.hintee].size(); ++i) {
        ret.positions[i] =
            hands[decision.hintee][i].colour() == decision.colour;
        if (ret.positions[i]) {
          any = true;
        }
      }
      assert(any);
      return ret;
    }
    case HINT_VALUE: {
      assert(numHints != 0);
      ret.hintee = decision.hintee;
      ret.value = decision.value;
      ret.positions.fill(false);
      bool any = false;
      for (int i = 0; i < hands[decision.hintee].size(); ++i) {
        ret.positions[i] = hands[decision.hintee][i].value() == decision.value;
        if (ret.positions[i]) {
          any = true;
        }
      }
      assert(any);
      return ret;
    }
  }
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
  switch (action.type) {
    case PLAY: {
      playCard(action.card);
      dropAndDraw(action.actor, action.pos, toDraw());
      break;
    }
    case DISCARD: {
      discardCard(action.card);
      dropAndDraw(action.actor, action.pos, toDraw());
      break;
    }
    case HINT_COLOUR:
    case HINT_VALUE: {
      numHints--;
      break;
    }
  }
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
