#include "SeerExecutor.h"
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include "Executor.h"
#include "Printer.h"
#include "common/logging/logging.h"
#include "util.h"

DEFINE_bool(allow_magic, true, "");
DEFINE_bool(always_brute_force, true, "");
DEFINE_bool(track_hand_weight, false, "");
DEFINE_bool(optimize_for_perfect, true, "");

namespace seer {

SeerDecision SeerDecision::Play(Card c) {
  return SeerDecision{SeerDecision::What::PLAY, c};
}

SeerDecision SeerDecision::Hint() {
  return SeerDecision{SeerDecision::What::HINT, Card()};
}

SeerDecision SeerDecision::Discard(Card c) {
  return SeerDecision{SeerDecision::What::DISCARD, c};
}

bool operator==(const SeerDecision& lhs, const SeerDecision& rhs) {
  return lhs.what == rhs.what && lhs.card == rhs.card;
}

std::vector<SeerDecision>
getOptions(Hand h, const Played& played, int numHints, bool allowBadDiscards) {
  std::vector<SeerDecision> ret;

  for (int i = 0; i < h.size(); i++) {
    bool isDupe = false;
    for (int j = 0; j < i; ++j) {
      if (h[i] == h[j]) {
        isDupe = true;
      }
    }
    Card c = h[i];
    if (!isDupe) {
      if (played.isPlayable(c)) {
        ret.push_back(SeerDecision::Play(c));
      }
    }
  }

  bool anyDiscardable = false;
  if (numHints < 8) {
    for (auto c : h) {
      if (played.isDiscardable(c)) {
        ret.push_back(SeerDecision::Discard(c));
        anyDiscardable = true;
        break;
      }
    }
    if (!anyDiscardable) {
      for (int i = 0; i < h.size(); ++i) {
        for (int j = i + 1; j < h.size(); ++j) {
          if (h[i] == h[j]) {
            ret.push_back(SeerDecision::Discard(h[i]));
            anyDiscardable = true;
            break;
          }
        }
      }
    }
  }

  if (numHints > 0) {
    ret.push_back(SeerDecision::Hint());
  }

  if (allowBadDiscards && !anyDiscardable && numHints < 8) {
    for (auto c : h) {
      ret.push_back(SeerDecision::Discard(c));
    }
  }

  return ret;
}

Decision
translate(const SeerDecision& sa, const GameState& g, bool underIsomorphism) {
  auto p = getPermutation(g).inverse;
  auto findCardIndex = [&](Card b) {
    if (underIsomorphism) {
      Card c(p[b.colour()], b.value());
      Hand h = g.hands[g.currentPlayer];
      for (int i = 0; i < h.size(); ++i) {
        if (h[i] == c) {
          return i;
        }
        if (b == Card(YELLOW, 1) && g.isDiscardable(h[i])) {
          return i;
        }
      }
      assert(false);
    }
    Hand h = g.hands[g.currentPlayer];
    for (int i = 0; i < h.size(); ++i) {
      if (h[i] == b) {
        return i;
      }
    }
    assert(false);
    __builtin_unreachable();
  };

  switch (sa.what) {
    case SeerDecision::What::PLAY:
      return Decision::play(findCardIndex(sa.card));
    case SeerDecision::What::DISCARD:
      return Decision::discard(findCardIndex(sa.card));
    case SeerDecision::What::HINT: {
      int who = (g.currentPlayer & 0x1) ^ 0x1;
      auto v = g.hands[who][0].value();
      return Decision::hint_value(who, v);
    }
  };
  assert(false);
}

SeerEndGameState::SeerEndGameState(const NaturesGameState& g) : GameState(g) {
  assert(deckSize <= 5);
  assert(deckSize == g.deck.size());
  auto deckCp = g.deck;
  for (int i = 0; i < deckSize; ++i) {
    Card c = deckCp.draw();
    assert(c != Card());
    deck[i] = c;
  }
  assert(deckCp.empty());
  for (int i = deckSize; i < 5; ++i) {
    deck[i] = Card();
  }

  reorder();
  yellowify();
  rotate();
  sort();
}

SeerEndGameState::SeerEndGameState(const SeerEndGameState& g) noexcept
    : GameState(g) {
  memcpy(this, &g, sizeof(SeerEndGameState));
}

SeerEndGameState SeerEndGameState::mk(const NaturesGameState& g) {
  SeerEndGameState* mem =
      reinterpret_cast<SeerEndGameState*>(malloc(sizeof(SeerEndGameState)));
  memset(mem, 0, sizeof(SeerEndGameState));
  new (mem) SeerEndGameState(g);
  SeerEndGameState ret(*mem);
  free(mem);
  return ret;
}

void SeerEndGameState::reorder() {
  auto p = getPermutation(*this);

  if (p.isIdentity) {
    return;
  }

  permute(p.permutation);

  for (auto& h : hands) {
    h = ::permute(p.permutation, h);
  }

  for (int i = 0; i < deckSize; ++i) {
    auto& c = deck[i];
    c = ::permute(p.permutation, c);
  }
}

void SeerEndGameState::yellowify() {
  for (auto c : uniqueCards()) {
    if (c.value() <= played(c)) {
      int d = numDiscarded(c);
      discard(c, -d);
      discard(Card(YELLOW, 1), d);
    }
  }

  for (auto& h : hands) {
    Hand yh;
    for (auto c : h) {
      if (c.value() <= played(c)) {
        yh.add(Card(YELLOW, 1));
      } else {
        yh.add(c);
      }
    }
    h = yh;
  }

  for (int i = 0; i < deckSize; ++i) {
    auto& c = deck[i];
    assert(c != Card());
    if (c.value() <= played(c)) {
      c = Card(YELLOW, 1);
    }
  }
}

void SeerEndGameState::rotate() {
  while (currentPlayer) {
    currentPlayer--;
    auto h0 = hands[0];
    for (int i = 1; i < numPlayers; ++i) {
      hands[i - 1] = hands[i];
    }
    hands[numPlayers - 1] = h0;
  }
}

void SeerEndGameState::sort() {
  std::sort(deck.begin(), deck.begin() + deckSize);
  for (auto& h : hands) {
    h = h.sorted();
  }
}

std::string SeerEndGameState::serialize() const {
  return std::string(
      reinterpret_cast<const char*>(this), sizeof(SeerEndGameState));
}

void SeerEndGameState::updateAndAdvance(
    const SeerDecision& decision,
    int nextCardIdx) {
  assert(actionsLeft != 0);
  if (actionsLeft > 0) {
    actionsLeft--;
  }
  auto toDraw = [&]() {
    if (deckSize == 0) {
      return Card();
    }
    Card ret = deck[nextCardIdx];
    for (int i = nextCardIdx + 1; i < deckSize; ++i) {
      deck[i - 1] = deck[i];
    }
    deckSize--;
    deck[deckSize] = Card();
    if (deckSize == 0) {
      actionsLeft = numPlayers;
    }
    return ret;
  };
  auto findPos = [this](Card c) {
    for (int i = 0; i < hands[0].size(); ++i) {
      if (hands[0][i] == c) {
        return i;
      }
    }
    assert(false);
    __builtin_unreachable();
  };

  switch (decision.what) {
    case SeerDecision::What::PLAY: {
      playCard(decision.card);
      dropAndDraw(0, findPos(decision.card), toDraw());
      reorder();
      yellowify();
      sort();
      break;
    }
    case SeerDecision::What::DISCARD: {
      discardCard(decision.card);
      dropAndDraw(0, findPos(decision.card), toDraw());
      sort();
      break;
    }
    case SeerDecision::What::HINT: {
      numHints--;
      break;
    }
  }
  advanceTurn();
  rotate();
}

// TODO critical analysis receives CardSet from util.h. CardSet gets count
// function
// TODO if possible, let people who have more playable cards be the hinters
// TODO don't discard 1s, even if that helps reduce criticals: pathological case
// where it takes forever to see the first 1
// TODO don't discard playable cards that won't otherwise be played

struct CriticalAnalysis {
  int8_t numUnreachable;
  int8_t numBlockedWeighted;
};
CriticalAnalysis analyzeCriticals(
    const Played& played,
    const Discarded& discarded) {
  int8_t numUnreachable = 0;
  int8_t numBlockedWeighted = 0;

  for (int c = 0; c < 5; ++c) {
    bool blocked = false;
    for (int v = played.played(c) + 1; v <= 5; ++v) {
      Card card(c, v);
      int d = discarded.numDiscarded(card);
      if (d == numCardsPerValue()[v]) {
        numUnreachable += 6 - v;
        break;
      }
      if (d) {
        numBlockedWeighted += 6 - v;
      }
    }
  }
  return {numUnreachable, numBlockedWeighted};
}

// TODO downrank when someone who has lots of critical cards is drawing more
// cards in the late game.
// TODO if everyone is hinting to wait for the green 1 to be played, someone
// damn well better have the green 2, otherwise we're wasting time
// TODO if someone has B4 and B5, and in dire straights another person discards
// their B4, then if we're stuck on blue then B4 B5 player cannot play both

class Ranking {
  enum Pos {
    INITIALIZED = 0,
    ACTIONS_NZ,
    NUM_UNREACHABLE,
    WINNABLE,
    NUM_NON5_BURIED_DISCARDED,
    SCORE_ORDER,
    SCORE_ORDER_2,
    SCORE_ORDER_3,
    SCORE_ORDER_4,
    SCORE_ORDER_5,
    SCORE,
    MAGIC,
    NUM_BLOCKED_WEIGHTED,
    NUM_HINTS,
    CRITICAL_MASS_DELTA,
    PLAYABLE_MASS_DELTA,
    LIMIT,
  };

 public:
  Ranking() {
    repr.fill(std::numeric_limits<int8_t>::min());
  }
  Ranking(
      const Played& _played,
      int numHints,
      const Discarded& _discarded,
      int actions)
      : played(_played), discarded(_discarded) {
    repr.fill(std::numeric_limits<int8_t>::min());
    repr[INITIALIZED] = true;
    repr[ACTIONS_NZ] = actions > 0;
    repr[WINNABLE] = discarded.numDiscarded() <= 13; // FIXME
    repr[SCORE] = played.numPlayed();
    repr[NUM_HINTS] = numHints;

    auto critAnalysis = analyzeCriticals(played, discarded);
    repr[NUM_NON5_BURIED_DISCARDED] = 0;
    repr[NUM_BLOCKED_WEIGHTED] = -critAnalysis.numBlockedWeighted;
    repr[NUM_UNREACHABLE] = -critAnalysis.numUnreachable;

    for (int i = 0; i < 5; ++i) {
      repr[SCORE_ORDER + i] = played.played(i);
    }
    std::sort(repr.data() + SCORE_ORDER, repr.data() + SCORE_ORDER + 5);

    // magic combines hints left, actions performed, and criticals made
    repr[MAGIC] = 0;
    if (FLAGS_allow_magic &&
        numHints > std::min<int>(13 - discarded.numDiscarded(), 3)) {
      repr[MAGIC] = -1;
    }

    repr[CRITICAL_MASS_DELTA] = 0;
    repr[PLAYABLE_MASS_DELTA] = 0;
  }

  bool isBad() const {
    return repr[MAGIC] != 0;
  }

  void addBlockingDiscard(Card c) {
    repr[NUM_NON5_BURIED_DISCARDED] -= (6 - c.value());
  }

  void discard(Hand h) {
    // TODO why does this not work?
    if (FLAGS_track_hand_weight) {
      // how many playable cards, and critical cards, are in this hand?
      int crits = 0;
      int playable = 0;
      for (auto c : h) {
        if (!played.isDiscardable(c)) {
          playable++;
          if (c.value() == 5 || discarded.numDiscarded(c)) {
            crits++;
          }
        }
      }

      repr[PLAYABLE_MASS_DELTA] -= playable * playable;
      repr[CRITICAL_MASS_DELTA] -= crits * crits;
    }
  }

  bool operator<(const Ranking& other) const {
    return repr < other.repr;
  }

 private:
  std::array<int8_t, LIMIT> repr;
  Played played;
  Discarded discarded;
};

bool operator<(const SeerDecision& lhs, const SeerDecision& rhs) {
  return lhs.what < rhs.what;
}

// TODO: unit tests for
//  bias to action when no one can do anything
// TODO prefer danger discarding a card whose sister is seen
// TODO don't penalize discarding of duplicates

std::pair<Ranking, SeerDecision> noDrawMaximize(
    const Hand* b,
    int off,
    const Hand* e,
    const Played& played,
    int numHints,
    const Discarded& discarded,
    int actions) {
  if (e - b == off) {
    return {Ranking(played, numHints, discarded, actions),
            SeerDecision::Hint()};
  }

  std::pair<Ranking, SeerDecision> ret;
  for (const auto& o : getOptions(b[off], played, numHints)) {
    auto p = played;
    auto h = numHints;
    auto d = discarded;
    auto a = actions;

    switch (o.what) {
      case SeerDecision::What::PLAY: {
        p.play(o.card);
        a++;
        break;
      }
      case SeerDecision::What::HINT: {
        h--;
        break;
      }
      case SeerDecision::What::DISCARD: {
        d.discard(o.card);
        h++;
        a++;
        break;
      }
    }

    auto nret = noDrawMaximize(b, off + 1, e, p, h, d, a);
    nret.second = o;

    if (o.what == SeerDecision::DISCARD && !played.isDiscardable(o.card)) {
      // check for discarding a card that isn't in anyone's hand
      // if it's in no one else's hand, then no one else could play it, so the
      // fact that we're using an intermediate played representation is ok
      int count = 0;
      for (auto* p = b; p != e; ++p) {
        for (auto c : *p) {
          if (c == o.card) {
            count++;
          }
        }
      }
      if (count == 1) {
        // we're about to discard the only copy!
        nret.first.addBlockingDiscard(o.card);
      }
      nret.first.discard(b[off]);
    }

    if (false && off == 0) {
      LOG(INFO) << (int)o.what << " "
                << folly::humanify(std::string(
                       reinterpret_cast<const char*>(&nret.first),
                       sizeof(nret.first)));
    }

    // By using the enum, with hints the lowest, we bias for action
    if (ret < nret) {
      ret = nret;
    }
  }
  return ret;
}

SeerDecision noDrawMaximize(const NaturesGameState& g) {
  auto mainOpts = getOptions(g.hands[g.currentPlayer], g, g.numHints, false);

  // some options are obvious; save computational power
  // no plays, discardable card, plenty of time left
  if (!mainOpts.empty() && mainOpts[0].what == SeerDecision::DISCARD &&
      g.numDiscarded() < 13) {
    return mainOpts[0];
  }
  // no plays, cannot discard
  if (!mainOpts.empty() && mainOpts[0].what == SeerDecision::HINT &&
      g.numHints == 8) {
    return mainOpts[0];
  }

  auto rot = [&](int i) {
    if (i >= g.numPlayers) {
      return i - g.numPlayers;
    }
    return i;
  };
  std::array<Hand, 5> orderedHands;
  for (int i = 0; i < g.numPlayers; ++i) {
    orderedHands[i] = g.hands[rot(g.currentPlayer + i)];
  }

  auto ans = noDrawMaximize(
      orderedHands.data(),
      0,
      orderedHands.data() + g.numPlayers,
      g,
      g.numHints,
      g,
      0);

  return ans.second;
}

namespace {

// state -> {decision, E(score)}
// folly::Synchronized<
//    std::unordered_map<std::string, std::pair<SeerDecision, double>>>
//    lookupTable;

using LookupTable =
    std::unordered_map<std::string, std::pair<SeerDecision, double>>;

std::pair<SeerDecision, double> endGameLookup(
    const SeerEndGameState& g,
    LookupTable& lookupTable);

std::pair<SeerDecision, double> endGameLookupImpl(
    const SeerEndGameState& g,
    LookupTable& lookupTable) {
  auto attempt = [&](const SeerEndGameState& g, SeerDecision d) {
    double acc = 0;
    int i = 0;
    for (; i < std::max<int>(1, g.deckSize); ++i) {
      auto gc = g;
      gc.updateAndAdvance(d, i);
      acc += endGameLookup(gc, lookupTable).second;
    }
    return acc / i;
  };

  std::pair<SeerDecision, double> ret;
  for (const auto& d : getOptions(g.hands[0], g, g.numHints)) {
    double escore = attempt(g, d);
    if (escore > ret.second) {
      ret.first = d;
      ret.second = escore;
    }
    if (escore == 25) {
      break;
    }
    if (g.deckSize == 0 && d.what != SeerDecision::PLAY) {
      // hinting/discarding is useless in the last round
      break;
    }
  }
  return ret;
}

std::pair<SeerDecision, double> endGameLookup(
    const SeerEndGameState& g,
    LookupTable& lookupTable) {
  assert(g.currentPlayer == 0);
  if (g.isOver()) {
    double value = g.score();
    if (FLAGS_optimize_for_perfect && value != 25) {
      value /= 10000;
    }
    return {SeerDecision::Hint(), value};
  }
  auto key = g.serialize();
  auto it = lookupTable.find(key);
  if (it != lookupTable.end()) {
    return it->second;
  }
  auto ret = endGameLookupImpl(g, lookupTable);
  lookupTable[key] = ret;
  return ret;
}

class SeerExecutor : public Executor {
 public:
  SeerExecutor(int numPlayers, unsigned seed) : Executor(numPlayers, seed) {}

  virtual Decision act(const NaturesGameState& g) {
    bool canWin = g.score() >= 25 - g.deckSize - g.numPlayers;
    bool shouldBruteForce = g.deckSize <= 5 &&
        (FLAGS_always_brute_force || canWin || g.deckSize == 0);
    if (shouldBruteForce) {
      auto decision = endGameLookup(SeerEndGameState::mk(g), lookupTable).first;
      return translate(decision, g, true);
    }

    auto d = noDrawMaximize(g);
    return translate(d, g, false);
  }

  LookupTable lookupTable;
};
} // namespace
} // namespace seer

std::unique_ptr<Executor> makeSeerExecutor(int numPlayers, unsigned seed) {
  return std::make_unique<seer::SeerExecutor>(numPlayers, seed);
}
