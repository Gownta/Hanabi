#include <glog/logging.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include "EndGame.h"
#include "Player.h"
#include "small_vector.h"

DEFINE_bool(enable_endgame, true, "");
DEFINE_bool(enable_nodrawsim, true, "");

// Seers can see their own hand
class Seer : public Player {
 public:
  Seer(const GameState& gs, int p, Token tok) : Player(gs, p, tok) {}

  Decision act() override {
    static std::array<int, 25> hintf;
    static std::array<int, 25> smallest_hintf;
    static int i = 0;
    if (++i % 1000000 == 0) {
      std::cerr << "Hintmap:\n";
      for (int i = 0; i < 25; ++i) {
        std::cerr << i << ":\t" << hintf[i] << ", " << smallest_hintf[i]
                  << "\n";
      }
    }
    small_vector<int, 5> hintn;
    for (int i = 0; i < game.numPlayers; ++i) {
      if (i != pos) {
        std::set<Value> values;
        std::set<Colour> colours;
        for (const auto& card : game.getHand(token, i)) {
          values.insert(card.value);
          colours.insert(card.colour);
        }
        hintn.push_back(values.size() + colours.size());
      }
    }
    smallest_hintf[*std::min_element(hintn.begin(), hintn.end())]++;
    hintf[std::accumulate(hintn.begin(), hintn.end(), 0)]++;

    if (FLAGS_enable_endgame && game.deck.size() <= 5 && game.score >= 21) {
      return endGamePlay(game, token, game.turn);
    }

    // if (FLAGS_enable_nodrawsim) {
    // return noDrawSimulation(game, token);
    //}

    const auto& hand = game.getHand(token, pos);

    // not technically true, but have two strong preferences:
    //  - prefer playing over discarding
    //  - prefer discarding discardable over regular over critical
    //
    // Simulate, without drawing, one round of play, optimizing for
    //  - score,
    //  - low cards played
    //  - critical cards played

    /*
        auto getPlayableIndices =
            [&](const Hand& hand, const std::array<Value, NUM_COLOURS>&
       played) { small_vector<int, 5> ret; for (int i = 0; i < hand.size();
       ++i) { const auto& card = hand[i]; if (played[card.colour] + 1 ==
       card.value) { ret.push_back(i);
                }
              }
              return ret;
            };
        auto canPlays = getPlayableIndices(hand, game.played);
        if (!canPlays.empty()) {
          // Can play. Do so. Which card is best?
          //  TODO there are cases where we shouldn't. For example, we can
       discard when another has just this card
          // Consideration:
          //  - lower value
          //  - future unlock
          //  - future unlock for otherwise unplayable player
          //  - 5, for hint gen
          //  - criticality

          // If only 1 card is playable, then easy choice
          if (canPlayIdx.size() == 1) {
            return PlayDecision{canPlayIdx[0]};
          }

          // Simulate everyone getting one round of play. Optimize for
          //  - total number of cards played
          //  - smaller piles getting played
          //  - critical cards getting played
          for (it

          // uh-oh! what do I do?
          // If someone else has the card, AND no one in between us has the
       next
          // card, AND that second person has no other playable cards, let
       them
          // play it


          // return the offset of the player. (pos + *ret) % NP
          std::optional<int> anotherHas = [&](const Card& card) {
            for (int j = 0; j < game.numPlayers - 1; ++j) {
              int i = (pos + 1 + j) % game.numPlayers;
              const auto& hand = game.getHand(token, i);
              for (const auto& c : hand) {
                if (card == c) {
                  return j;
                }
              }
            }
            return {};
          };

          auto countPlayables = [&](int pos) {
            int ret = 0;
            for (const auto& card : game.getHand(token, pos)) {
              if (game.playableBits.has(card)) {
                ret++;
              }
            }
            return ret;
          };

          small_vector<int, 5> scores;
          for (auto idx : canPlayIdx) {
            const auto& card = hand[idx];

            // lower is better
            int score = 5 - card.value;

            // let others play if they can
            auto whoSame = anotherHas(card);
            Card next = card;
            next.value++;
            auto whoNext = anotherHas(next);

            if (whoSame) {
              if (!whoNext || *whoSame
            }
            if (who) {
              score -= 1;
              if (countPlayables(*who) == 1) {
                // That person only has this one thing to do. Let them do it
                score -= 10;
              }
            }
          }
        }
        */
    int toPlay = hand.size();
    for (int i = 0; i < hand.size(); ++i) {
      if (!game.playableBits.has(hand[i])) {
        continue;
      }
      if (toPlay == hand.size()) {
        toPlay = i;
        continue;
      }

      // lower value
      if (hand[i].value < hand[toPlay].value) {
        toPlay = i;
      }
    }
    if (toPlay != hand.size()) {
      return PlayDecision{toPlay};
    }

    if (game.numHints == STARTING_HINTS) {
      int who = (pos + 1) % 4;
      Value what = game.getHand(token, who)[0].value;
      return ValueHintDecision{who, what};
    }

    int numPlays = 0;
    for (int i = 0; i < game.numPlayers; ++i) {
      numPlays += (bool)countPlayables(game.getHand(token, i));
    }
    int numDiscards = 0;
    for (int i = 0; i < game.numPlayers; ++i) {
      numDiscards += (bool)countSafeDiscards(game.getHand(token, i));
    }
    if (numPlays > 0 && game.deck.size() < game.numHints) {
      int who = (pos + 1) % 4;
      Value what = game.getHand(token, who)[0].value;
      return ValueHintDecision{who, what};
    }

    auto safeD = safeDiscardPos(hand);
    if (safeD) {
      return DiscardDecision{*safeD};
    }

    int highValPos = -1; // highest-valued card in hand
    int highNCPos = -1; // non-critical position, highest value
    for (int i = 0; i < hand.size(); ++i) {
      const auto& card = hand[i];
      if (game.criticalBits.has(card)) {
        if (highValPos == -1 || card.value > hand[highValPos].value) {
          highValPos = i;
        }
      } else {
        if (highNCPos == -1 || card.value > hand[highNCPos].value) {
          highNCPos = i;
        }
      }
    }
    if (game.numHints < 2 && highNCPos != -1) {
      return DiscardDecision{highNCPos};
    }
    if (game.numHints == 0) {
      return DiscardDecision{highValPos};
    }

    // discard only if I can discard a high-valued non-critical card,
    // where high is relative to the number of other players who can play
    if (highNCPos != -1 &&
        hand[highNCPos].value - (numPlays + numDiscards) >= 3) {
      return DiscardDecision{highNCPos};
    }
    int who = (pos + 1) % 4;
    Value what = game.getHand(token, who)[0].value;
    return ValueHintDecision{who, what};
  }

  int countPlayables(const Hand& hand) {
    int ret = 0;
    for (const auto& card : hand) {
      ret += game.playableBits.has(card);
    }
    return ret;
  }

  int countSafeDiscards(const Hand& hand) {
    int ret = 0;
    for (const auto& card : hand) {
      ret += game.discardableBits.has(card);
    }
    return ret;
  }

  std::optional<int> safeDiscardPos(const Hand& hand) {
    for (int i = 0; i < hand.size(); ++i) {
      if (game.discardableBits.has(hand[i])) {
        return i;
      }
    }
    for (int i = 0; i < hand.size(); ++i) {
      for (int j = i + 1; j < hand.size(); ++j) {
        if (hand[i] == hand[j]) {
          return i;
        }
      }
    }
    return {};
  }

  Decision simpleAct() {
    const auto& hand = game.getHand(token, pos);
    for (int i = 0; i < hand.size(); ++i) {
      const auto& card = hand[i];
      if (game.played[card.colour] + 1 == card.value) {
        return PlayDecision{i};
      }
    }
    if (game.numHints < STARTING_HINTS) {
      for (int i = 0; i < hand.size(); ++i) {
        const auto& card = hand[i];
        if (game.played[card.colour] >= card.value) {
          return DiscardDecision{i};
        }
      }
    }
    if (game.numHints > 0) {
      int who = (pos + 1) % 4;
      Value what = game.getHand(token, who)[0].value;
      return ValueHintDecision{who, what};
    }
    return DiscardDecision{0};
  }
};

std::unique_ptr<Player> createSeer(const GameState& gs, int p, Token tok) {
  return std::make_unique<Seer>(gs, p, tok);
}
