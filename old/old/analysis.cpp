#include "analysis.h"
#include "GameState.h"

double pUnwinnable(int numPlayers, int iterations) {
  int cardsPerPlayer = numPlayers <= 3 ? 5 : 4;
  int inHands = numPlayers * cardsPerPlayer;
  int maxDiscard = 25 - numPlayers * (cardsPerPlayer - 1);
  int additional = inHands + maxDiscard;
  int bad = 0;

  for (int seed = 0; seed < iterations; ++seed) {
    Deck d(seed);
    Played p;
    CardSet s;
    for (int i = 0; i < 50; ++i) {
      Card c = d.draw();
      s.add(c);
      while (p.isPlayable(c) && s.has(c)) {
        p.play(c);
        if (c.value() == 5) {
          break;
        }
        c = Card(c.colour(), c.value() + 1);
      }

      if (i + 1 - p.numPlayed() > additional) {
        bad++;
        goto CML;
      }
    }

  CML:;
  }

  return 1.0 * bad / iterations;
}
