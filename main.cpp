#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>
#include <vector>
#include "Executor.h"
#include "common/init/Init.h"
#include "common/logging/logging.h"
#include "common/time/ChronoFlags.h"
#include "common/time/Time.h"
#include "seeds.h"

DEFINE_int32(threads, 48, "");
DEFINE_int32(record_score, 0, "Print games less than or equal this score");
DEFINE_time_s(record_time, 60_s, "Print games which take longer than this");
DEFINE_bool(analyze, false, "");

int main(int argc, char** argv) {
  // Flags and logging
  facebook::initFacebook(&argc, &argv);
  LOG(INFO) << "\n\
========================================\n\
Starting hanabi executable\n\
========================================";

  // Global initialization
  initCards(false);

  // Play games, either one interactive or all in parallel
  auto seeds = getSeeds();
  LOG(INFO) << "Playing " << seeds.size() << " games";
  if (seeds.size() == 1) {
    auto executor = makeSeerExecutor(4, seeds[0]);
    executor->play(true);
  } else {
    std::atomic<int> gamesPlayed = 0;
    std::atomic<int> totalScore = 0;
    std::atomic<int> perfectGames = 0;
    auto evaluator = [&]() {
      for (;;) {
        int gameIdx = gamesPlayed++;
        if (gameIdx >= seeds.size()) {
          return;
        }
        facebook::WallTimer t{true};
        auto executor = makeSeerExecutor(4, seeds[gameIdx]);
        int score = executor->play(false);
        auto time = t.get();
        LOG_IF(
            INFO,
            score <= FLAGS_record_score || time >= FLAGS_record_time_s.count())
            << "Game " << seeds[gameIdx] << " scored " << score << ", taking "
            << time << "s";
        totalScore += score;
        if (score == 25) {
          perfectGames++;
        }
      }
    };
    std::vector<std::thread> threads;
    for (int i = 0; i < FLAGS_threads; ++i) {
      threads.emplace_back(evaluator);
    }
    for (auto& t : threads) {
      t.join();
    }
    LOG(INFO) << "Done playing. Average score: "
              << ((double)totalScore / seeds.size()) << " across "
              << seeds.size() << " games, with "
              << (100.0 * perfectGames / seeds.size()) << "\% perfect";
  }

  return 0;
}
