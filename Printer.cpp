#include "Printer.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
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

std::ostream& operator<<(std::ostream& os, Card card) {
  return os << colourCode(card.colour()) << (int)card.value() << "\033[0m";
}

void printCard(const Card& card) {
  cout << card;
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

void output(int pos, int currPlayer, Hand hand, int down, int right) {
  cursor(down, right);
  if (pos == currPlayer) {
    cout << "\033[45m";
  }
  cout << "Player " << pos;
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

void printTable(const PublicGameState& state, int down, int right) {
  cursor(down, right);
  cout << "Played      Discarded";
  for (int i = 0; i < colours().size(); ++i) {
    cursor(down + i + 1, right + 1);
    for (int j = 1; j <= state.played(colours()[i]); ++j) {
      printCard(Card(colours()[i], j));
      cout << ' ';
    }
    cursor(down + i + 1, right + 13);
    for (int j = 1; j <= 5; ++j) {
      for (int k = 0; k < state.numDiscarded(Card(colours()[i], j)); ++k) {
        printCard(Card(colours()[i], j));
        cout << ' ';
      }
    }
  }
}

void printAction(const Action& action) {
  switch (action.type) {
    case PLAY: {
      cout << "[P] " << action.actor << "   played    position " << action.pos
           << "   ";
      printCard(action.card);
      return;
    }
    case DISCARD: {
      cout << "[D] " << action.actor << "   discards  position " << action.pos
           << "   ";
      printCard(action.card);
      return;
    }
    case HINT_COLOUR: {
      cout << "[H] " << action.actor << "   hinted    player " << action.hintee
           << "   about " << text(action.colour);
      return;
    }
    case HINT_VALUE: {
      cout << "[H] " << action.actor << "   hinted    player " << action.hintee
           << "   about " << (int)action.value;
      return;
    }
  }
}

void printActions(
    const Action* b,
    const Action* e,
    int maxActionsToDisplay,
    int down,
    int right) {
  if (e - b > maxActionsToDisplay) {
    b = e - maxActionsToDisplay;
  }
  for (int off = 0; b != e; ++b, off++) {
    cursor(down + off, right);
    printAction(*b);
  }
}

void printDeck(const NaturesGameState& g, int down, int right) {
  cursor(down, right);
  auto cp = g.deck;
  cout << "Deck:";
  while (!cp.empty()) {
    cout << " " << cp.draw();
  }
}

void printGame(const NaturesGameState& g, const Action* b, const Action* e) {
  assert(g.numPlayers == 4);
  clear();

  // counters
  int score = g.score();
  cout << "Seed: " << g.seed << "\n"
       << "Deck: " << (int)g.deckSize << "\n"
       << "Hints: " << (int)g.numHints << "\n"
       << "Bombs: " << (int)g.numBombsLeft << "\n"
       << "Score: " << (int)score << "\n"
       << "Turn: " << (int)g.turn << "\n";

  // players
  output(0, g.currentPlayer, g.hands[0], 15, 30);
  output(1, g.currentPlayer, g.hands[1], 8, 5);
  output(2, g.currentPlayer, g.hands[2], 2, 30);
  output(3, g.currentPlayer, g.hands[3], 8, 55);

  printTable(g, 7, 25);
  printActions(b, e, 20, 1, 70);
  printDeck(g, 21, 0);

  cursor(23, 0);
  std::cout.flush();
}
