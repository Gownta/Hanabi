#include <array>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "Executor.h"
#include "Player.h"
#include "common/init/Init.h"
#include "common/logging/logging.h"
#include "small_vector.h"
//#include "players/Knoweldge.h"

DEFINE_int32(ngames, 0, "");
DEFINE_int32(seed, 0, "");
DEFINE_int32(threads, 48, "");
DEFINE_int32(record_score, 0, "Print games le this score");

int main(int argc, char** argv) {
  // Flags and logging
  facebook::initFacebook(&argc, &argv);
  LOG(INFO) << "\n\
========================================\n\
Starting hanabi executable\n\
========================================";

  // Global initialization
  // -- nothing so far --

  // Play games
  bool interactive = FLAGS_ngames == 0;
  int ngames = std::max(FLAGS_ngames, 1);
  std::atomic<int> gamesPlayed = 0;
  std::atomic<int> totalScore = 0;
  auto evaluator = [&]() {
    for (;;) {
      int gameIdx = gamesPlayed++;
      if (gameIdx >= ngames) {
        return;
      }
      int seed = FLAGS_seed + gameIdx;
      auto playerBuilder = [&](const GameState& game, int i) {
        Token seerToken(4, seed);
        return createSeer(game, i, seerToken);
      };
      int score = execute(4, playerBuilder, seed, interactive);
      VLOG_IF(1, score <= FLAGS_record_score)
          << "Game " << seed << " scored " << score;
      totalScore += score;
    }
  };
  std::vector<std::thread> threads;
  for (int i = 0; i < FLAGS_threads; ++i) {
    threads.emplace_back(evaluator);
  }
  for (auto& t : threads) {
    t.join();
  }
  std::cout << "Average score: " << ((double)totalScore / ngames) << " across "
            << ngames << " games" << std::endl;

  return 0;
}
