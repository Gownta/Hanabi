#include "GameState.h"
#include <glog/logging.h>
#include <cassert>

int tokHash(int h, int seed) {
  int ret = (0x4ad77 ^ seed) ^ h;
  return ret;
}
Token::Token(int playerPos, unsigned seed)
    : pos(playerPos), tok(tokHash(playerPos, seed)) {}

GameState::GameState(int numP, unsigned seed)
    : deck(seed),
      numPlayers(numP),
      playableBits(1),
      criticalBits(5),
      seed_(seed) {
  for (int i = 0; i < numPlayers; ++i) {
    hands_.push_back(Hand());
    for (int j = 0; j < CARDS_PER_PLAYER[numPlayers]; ++j) {
      hands_[i].push_back(deck.draw());
    }
  }

  played.fill(0);
  discarded.fill(0);
}

const Hand& GameState::getHand(const Token& token, int pos) const {
  int tokp = tokHash(token.tok, seed_);
  assert(tokp == token.pos);
  assert(0 <= tokp && tokp <= numPlayers); // == for omniscient token
  if (tokp != pos) {
    return hands_[pos];
  }
  assert(false);
  __builtin_unreachable();
}

int GameState::getHandSize(int pos) const {
  return hands_[pos].size();
}

bool GameState::isOver() const {
  return turn >= lastTurn || score == 25 || numBombsLeft == 0;
}

void GameState::advanceTurn() {
  currentPlayer++;
  if (currentPlayer == numPlayers) {
    currentPlayer = 0;
  }
  turn++;
}

Action GameState::resolve(int player, const Decision& decision) {
  assert(!decision.valueless_by_exception());

  if (const auto* p = std::get_if<PlayDecision>(&decision)) {
    return PlayAction{player, p->pos, hands_[player][p->pos]};
  }
  if (const auto* p = std::get_if<DiscardDecision>(&decision)) {
    return DiscardAction{player, p->pos, hands_[player][p->pos]};
  }
  if (const auto* p = std::get_if<ColourHintDecision>(&decision)) {
    ColourHintAction ret{player, p->player, p->colour};
    bool any = false;
    for (int i = 0; i < 4; ++i) {
      ret.positions[i] = hands_[p->player][i].colour == p->colour;
      if (ret.positions[i]) {
        any = true;
      }
    }
    assert(any);
    return ret;
  }
  if (const auto* p = std::get_if<ValueHintDecision>(&decision)) {
    ValueHintAction ret{player, p->player, p->value};
    bool any = false;
    for (int i = 0; i < 4; ++i) {
      ret.positions[i] = hands_[p->player][i].value == p->value;
      if (ret.positions[i]) {
        any = true;
      }
    }
    assert(any);
    return ret;
  }
  assert(false);
  __builtin_unreachable();
}

void GameState::update(const Action& action) {
  assert(!action.valueless_by_exception());

  if (const auto* p = std::get_if<PlayAction>(&action)) {
    const Card& card = p->card;
    if (played[card.colour] + 1 == card.value) {
      cardToTable(card);
    } else {
      numBombsLeft--;
      cardToDiscard(card);
    }
    dropAndDraw(p->actor, p->pos);
  } else if (const auto* p = std::get_if<DiscardAction>(&action)) {
    assert(numHints != STARTING_HINTS);
    numHints++;
    cardToDiscard(p->card);
    dropAndDraw(p->actor, p->pos);
  } else if (const auto* p = std::get_if<ColourHintAction>(&action)) {
    assert(numHints);
    numHints--;
  } else if (const auto* p = std::get_if<ValueHintAction>(&action)) {
    assert(numHints);
    numHints--;
  } else {
    assert(false);
    __builtin_unreachable();
  }
  actions.push_back(action);
}

void GameState::cardToTable(const Card& card) {
  played[card.colour]++;
  score++;
  if (card.value == 5 && numHints < STARTING_HINTS) {
    numHints++;
  }
  discardableBits.add(card);
  playableBits.remove(card);
  if (card.value != 5) {
    playableBits.add(Card(card.colour, card.value + 1));
  }
  criticalBits.remove(card);
}

void GameState::cardToDiscard(const Card& card) {
  int nd = ++discarded[card.index()];
  if (nd == CARDS_PER_VALUE[card.value] - 1 && !discardableBits.has(card)) {
    criticalBits.add(card);
  } else if (nd == CARDS_PER_VALUE[card.value]) {
    for (int v = card.value; v <= 5; ++v) {
      criticalBits.remove(Card(card.colour, v));
      discardableBits.add(Card(card.colour, v));
    }
    maxScore--;
    for (int v = card.value + 1; v <= 5; ++v) {
      if (discarded[Card(card.colour, v).index()] == CARDS_PER_VALUE[v]) {
        break;
      }
      maxScore--;
    }
  }
}

void GameState::dropAndDraw(int player, int pos) {
  hands_[player].erase(pos);
  if (!deck.empty()) {
    hands_[player].push_back(deck.draw());
    if (deck.empty()) {
      lastTurn = turn + numPlayers;
    }
  }
}
