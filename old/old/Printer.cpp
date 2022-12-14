#include "Printer.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const std::string& colourCode(Colour c) {
  // static array<string, 6> lookup = {
  //    "\033[90m", "\033[96m", "\033[92m", "\033[91m", "\033[97m", "\033[93m"};
  static array<string, 6> lookup = {"\033[7;49;90m",
                                    "\033[7;49;96m",
                                    "\033[7;49;92m",
                                    "\033[7;49;91m",
                                    "\033[7;49;97m",
                                    "\033[7;49;93m"};
  return lookup[c + 1];
}

const std::string& text(Colour c) {
  static array<string, 6> lookup = {
      "?",
      "blue",
      "green",
      "red",
      "white",
      "yellow",
  };
  return lookup[c + 1];
}

void printCard(const Card& card) {
  cout << colourCode(card.colour) << (int)card.value << "\033[0m";
}

void cursor(int down, int right) {
  cout << "\033[" << down << ';' << right << "H";
}

void reset() {
  cout << "\033[0m";
}

void clear() {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

  cursor(0, 0);
  string blank(size.ws_col, ' ');
  for (int i = 0; i < size.ws_row - 1; ++i) {
    cout << blank << '\n';
  }
  cout << blank;
  cursor(0, 0);
  reset();
}

void output(int pos, int currPlayer, const Hand& hand, int down, int right) {
  cursor(down, right);
  if (pos == currPlayer) {
    cout << "\033[45m";
  }
  cout << "Player " << (pos + 0);
  reset();

  cursor(down + 1, right + 1);
  cout << "\033[90m 0  1  2  3 \033[0m";
  cursor(down + 2, right + 1);
  for (int i = 0; i < hand.size(); ++i) {
    cout << ' ';
    printCard(hand[i]);
    cout << ' ';
  }
}

void printTable(const GameState& state, int down, int right) {
  cursor(down, right);
  cout << "Played      Discarded";
  for (int i = 0; i < Colours.size(); ++i) {
    cursor(down + i + 1, right + 1);
    for (int j = 1; j <= state.played[Colours[i]]; ++j) {
      printCard(Card(Colours[i], j));
      cout << ' ';
    }
    cursor(down + i + 1, right + 13);
    for (int j = 0; j < 5; ++j) {
      for (int k = 0; k < state.discarded[5 * Colours[i] + j]; ++k) {
        printCard(Card(Colours[i], j + 1));
        cout << ' ';
      }
    }
  }
}

void printAction(const Action& action) {
  assert(!action.valueless_by_exception());
  if (const auto* p = std::get_if<PlayAction>(&action)) {
    cout << "[P] player " << (p->actor + 0) << "   played    position "
         << p->pos << "   ";
    printCard(p->card);
  }
  if (const auto* p = std::get_if<DiscardAction>(&action)) {
    cout << "[D] player " << (p->actor + 0) << "   discarded position "
         << p->pos << "   ";
    printCard(p->card);
  }
  if (const auto* p = std::get_if<ColourHintAction>(&action)) {
    cout << "[C] player " << (p->actor + 0) << "   hinted    player "
         << (p->hintee + 0) << "   about " << text(p->colour);
  }
  if (const auto* p = std::get_if<ValueHintAction>(&action)) {
    cout << "[C] player " << (p->actor + 0) << "   hinted    player "
         << (p->hintee + 0) << "   about " << (int)p->value;
  }
}

void printActions(
    const GameState& state,
    int maxActionsToDisplay,
    int down,
    int right) {
  const auto& actions = state.actions;
  int nactions = actions.size();
  for (int start = std::max(0, nactions - maxActionsToDisplay), i = start;
       i < nactions;
       ++i) {
    cursor(down + i - start, right);
    printAction(actions[i]);
  }
}

void printGame(const GameState& state, const Token& tok) {
  assert(state.numPlayers == 4);
  clear();

  // counters
  cout << "Deck: " << state.deck.size() << "\n"
       << "Hints: " << state.numHints << "\n"
       << "Bombs: " << state.numBombsLeft << "\n"
       << "Score: " << state.score << "\n"
       << "Turn: " << state.turn << "\n";

  // players
  output(0, state.currentPlayer, state.getHand(tok, 0), 15, 30);
  output(1, state.currentPlayer, state.getHand(tok, 1), 8, 5);
  output(2, state.currentPlayer, state.getHand(tok, 2), 2, 30);
  output(3, state.currentPlayer, state.getHand(tok, 3), 8, 55);

  // table
  printTable(state, 7, 25);

  // actions
  printActions(state, 20, 1, 80);

  cursor(23, 0);
}
