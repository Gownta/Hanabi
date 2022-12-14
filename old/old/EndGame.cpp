#include "EndGame.h"
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <glog/logging.h>
#include <algorithm>
#include <cstring>
#include <numeric>
#include <set>
#include <string>
#include <unordered_map>

namespace {
constexpr int MAX_DECK_SIZE = 5;

enum PHD : int8_t {
  PLAY,
  HINT,
  DISCARD,
};
struct What {
  PHD phd;
  Card card;
};

std::array<Colour, 5> inv(const std::array<Colour, 5>& in) {
  auto ret = in;
  for (int i = 0; i < 5; ++i) {
    ret[in[i]] = (Colour)i;
  }
  return ret;
}

// inv(return)[blue] is what blue transforms into
std::array<Colour, 5> invreord(const std::array<Value, 5>& values) {
  std::array<Colour, 5> sm{BLUE, GREEN, RED, WHITE, YELLOW};
  std::stable_sort(sm.begin(), sm.end(), [&](Colour c1, Colour c2) {
    return values[c1] < values[c2];
  });
  return sm;
}

struct GameRepr {
  int numHints;
  int numBombs;
  int turnsLeft;

  small_vector<Card, MAX_DECK_SIZE> deck;
  std::array<Value, 5> played;
  small_vector<Hand, 5> hands;
  std::array<int8_t, NUM_UNIQUE_CARDS> discarded;

  void updatePlay(int pos) {
    const auto& toPlay = hands[0][pos];
    assert(played[toPlay.colour] + 1 == toPlay.value);

    played[toPlay.colour]++;
    if (toPlay.value == 5 && numHints < STARTING_HINTS) {
      numHints++;
    }
    hands[0].erase(pos);
  }

  void updateDiscard(int pos) {
    assert(numHints < STARTING_HINTS);
    numHints++;
    const auto& toDiscard = hands[0][pos];
    discarded[toDiscard.index()]++;
    hands[0].erase(pos);
  }

  void updateHint() {
    assert(numHints > 0);
    numHints--;
  }

  void advanceTurn(int i) {
    assert(deck.size() > i);
    const auto& draw = deck[i];
    hands[0].push_back(draw);
    deck.erase(i);

    rotate();
    relabel();
    nullify();
    sort();

    if (deck.size() == 0) {
      turnsLeft = 3;
    }
  }

  // return true if we advanced, false if the game is over
  void advanceTurn() {
    if (turnsLeft != 1000) {
      turnsLeft--;
    }

    rotate();
    relabel();
    nullify();
    sort(); // not necessarily sorted due to relabeling and nullification
  }

  static GameRepr mk(const GameState& game, const Token& omni) {
    GameRepr g;
    std::memset(&g, 0, sizeof(GameRepr));

    g.numHints = game.numHints;
    g.numBombs = game.numBombsLeft;
    if (!game.deck.empty()) {
      g.turnsLeft = 1000;
    } else {
      g.turnsLeft = game.lastTurn - game.turn;
    }

    auto deckCp = game.deck;
    while (!deckCp.empty()) {
      g.deck.push_back(deckCp.draw());
    }
    g.played = game.played;
    for (int i = 0; i < game.numPlayers; ++i) {
      g.hands.push_back(game.getHand(omni, i));
    }
    g.discarded = game.discarded;

    g.rotate(game.currentPlayer);
    g.relabel();
    g.nullify();
    g.sort();

    return g;
  }

  // makes player zero the current player
  void rotate(int spots = 1) {
    for (; spots > 0; --spots) {
      auto hand = hands[0];
      hands.erase(0);
      hands.push_back(hand);
    }
  }

  // relabels colours such that played[BLUE]<=...<=played[YELLOW]
  void relabel() {
    auto p = inv(invreord(played));
    if (p == std::array<Colour, 5>{BLUE, GREEN, RED, WHITE, YELLOW}) {
      return;
    }

    auto newPlayed = played;
    for (int i = 0; i < 5; ++i) {
      newPlayed[p[i]] = played[i];
    }
    played = newPlayed;

    for (auto& card : deck) {
      card.colour = p[card.colour];
    }
    for (auto& hand : hands) {
      for (auto& card : hand) {
        card.colour = p[card.colour];
      }
    }

    auto newDiscarded = discarded;
    for (const auto& card : allCards()) {
      if (p[card.colour] == card.colour) {
        continue;
      }
      Card mapped(p[card.colour], card.value);
      newDiscarded[mapped.index()] = discarded[card.index()];
    }
    discarded = newDiscarded;
  }

  // turn all useless cards into yellow 1s
  // don't bother with unreachable cards; negative complexity utility
  void nullify() {
    Card y1(YELLOW, 1); // has been played; we have more than 0 points

    for (auto& card : deck) {
      if (card.value <= played[card.colour]) {
        card = y1;
      }
    }
    for (auto& hand : hands) {
      for (auto& card : hand) {
        if (card.value <= played[card.colour]) {
          card = y1;
        }
      }
    }
    for (const auto& card : allCards()) {
      if (card.value <= played[card.colour]) {
        discarded[card.index()] = 0;
      }
    }
  }

  // sorts hands and deck
  void sort() {
    std::sort(deck.begin(), deck.end());
    for (auto& hand : hands) {
      std::sort(hand.begin(), hand.end());
    }
  }

  std::string serialize() const {
    return std::string((const char*)this, sizeof(GameRepr));
  }

  GameRepr(const GameRepr& other) {
    std::memcpy((char*)this, &other, sizeof(GameRepr));
  }

 private:
  GameRepr() {}
};

// score, What
folly::Synchronized<std::unordered_map<std::string, std::pair<double, What>>>
    cache;
std::pair<double, What> solve(const GameRepr& repr, int code) {
  SYNCHRONIZED_CONST(cache) {
    // LOG_EVERY_N(INFO, 100000)
    //    << "Solving another game. Total solved = " << cache.size();
    auto cit = cache.find(repr.serialize());
    if (cit != cache.end()) {
      return cit->second;
    }
  }

  if (false && code == 36) {
    for (int i = 0; i < 4; ++i)
      assert(repr.hands[0][i] == Card(YELLOW, 1));
  }

  std::pair<double, What> ret{0, {PLAY, Card(YELLOW, 1)}};
  auto up = [&](double newScore, What what) {
    if (ret.first - (ret.second.phd == HINT ? 0.001 : 0) <= newScore) {
      ret.first = newScore;
      ret.second = what;
    }
  };

  if (repr.turnsLeft < 0) {
    ret.first = std::accumulate(repr.played.begin(), repr.played.end(), 0);
    return ret; // don't even bother caching it
  }

  if (repr.numHints > 0) {
    auto rcp = repr;
    rcp.updateHint();
    rcp.advanceTurn();
    if (false && code == 36) {
      LOG(INFO) << "Representation for hinting in code " << code << ":\n"
                << folly::humanify(rcp.serialize());
    }
    up(solve(rcp, code + 1).first, {HINT, Card(YELLOW, 1)});
  }
  if (repr.numHints < STARTING_HINTS) {
    for (int pos = 0; pos < repr.hands[0].size(); ++pos) {
      if (repr.deck.empty()) {
        auto rcp = repr;
        rcp.updateDiscard(pos);
        rcp.advanceTurn();
        up(solve(rcp, code + 1).first, {DISCARD, repr.hands[0][pos]});
      } else {
        double sum = 0;
        for (int i = 0; i < repr.deck.size(); ++i) {
          auto rcp = repr;
          rcp.updateDiscard(pos);
          rcp.advanceTurn(i);
          if (false && code == 36) {
            LOG(INFO) << "Representation for discarding " << i << " in code "
                      << code << ":\n"
                      << folly::humanify(rcp.serialize());
          }
          sum += solve(rcp, code + 1).first;
        }
        up(sum / repr.deck.size(), {DISCARD, repr.hands[0][pos]});
      }
    }
  }
  for (int pos = 0; pos < repr.hands[0].size(); ++pos) {
    const Card& card = repr.hands[0][pos];
    if (repr.played[card.colour] + 1 != card.value) {
      continue;
    }

    if (repr.deck.empty()) {
      auto rcp = repr;
      rcp.updatePlay(pos);
      rcp.advanceTurn();
      up(solve(rcp, code + 1).first, {PLAY, card});
    } else {
      double sum = 0;
      for (int i = 0; i < repr.deck.size(); ++i) {
        auto rcp = repr;
        rcp.updatePlay(pos);
        rcp.advanceTurn(i);
        if (false && code == 36) {
          LOG(INFO) << "Representation for playing " << i << " in code " << code
                    << ":\n"
                    << folly::humanify(rcp.serialize());
        }
        sum += solve(rcp, code + 1).first;
      }
      up(sum / repr.deck.size(), {PLAY, repr.hands[0][pos]});
    }
  }

  SYNCHRONIZED(cache) {
    cache.insert({repr.serialize(), ret});
  }
  return ret;
}

} // namespace

Decision endGamePlay(const GameState& game, const Token& omni, int code) {
  assert(game.deck.size() <= MAX_DECK_SIZE);
  assert(game.score >= 17);

  auto repr = GameRepr::mk(game, omni);
  if (false && code == 37) {
    LOG(INFO) << "Representation for solve at code " << code << ":\n"
              << folly::humanify(repr.serialize());
  }
  auto todo = solve(repr, code).second;
  int sz = 0;
  SYNCHRONIZED_CONST(cache) {
    sz = cache.size();
  }
  Colour& c = todo.card.colour;
  c = invreord(game.played)[c];

  if (todo.phd == HINT) {
    int who = (game.currentPlayer + 1) % 4;
    Value what = game.getHand(omni, who)[0].value;
    return ValueHintDecision{who, what};
  }

  const auto& hand = game.getHand(omni, game.currentPlayer);
  for (int i = 0; i < hand.size(); ++i) {
    if (todo.card == hand[i]) {
      if (todo.phd == PLAY) {
        return PlayDecision{i};
      } else {
        return DiscardDecision{i};
      }
    }
    if (todo.phd == DISCARD && game.discardableBits.has(hand[i])) {
      return DiscardDecision{i};
    }
  }
  LOG(ERROR) << "Instructions were " << todo.phd << " " << (int)todo.card.colour
             << (int)todo.card.value << " for " << game.seed_;
  assert(false);
}
