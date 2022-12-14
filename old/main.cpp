#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>
#include <vector>
#include "Executor.h"
#include "analysis.h"
#include "common/init/Init.h"
#include "common/logging/logging.h"
#include "common/time/ChronoFlags.h"
#include "common/time/Time.h"

DEFINE_int32(ngames, 0, "");
DEFINE_int32(seed, 0, "");
DEFINE_int32(threads, 48, "");
DEFINE_int32(record_score, 0, "Print games le this score");
DEFINE_time_s(record_time, 60_s, "Print games which take longer than this");
DEFINE_string(seed_file, "", "Read seeds from this file");
DEFINE_bool(analyze, false, "");

std::vector<unsigned> getSeeds() {
  std::vector<unsigned> ret;

  if (FLAGS_seed_file == "" || FLAGS_seed != 0 || FLAGS_ngames != 0) {
    for (int i = 0; i < std::max<int>(FLAGS_ngames, 1); ++i) {
      ret.push_back(i + FLAGS_seed);
    }
    return ret;
  }

  std::ifstream input(FLAGS_seed_file, std::ios::binary);
  for (;;) {
    unsigned next = 0;
    input >> next;
    if (next == 0) {
      break;
    }
    ret.push_back(next);
  }
  return ret;
}

int main(int argc, char** argv) {
  // Flags and logging
  facebook::initFacebook(&argc, &argv);
  LOG(INFO) << "\n\
========================================\n\
Starting hanabi executable\n\
========================================";

  // Global initialization
  initCards(false);

  if (FLAGS_analyze) {
    auto p1 = pUnwinnable(4, 100000) * 100.0;
    LOG(INFO) << "Cannot win at least " << p1 << "\% of 4-player games";
    return 0;
  }

  // Play games
  auto seeds = getSeeds();
  LOG(INFO) << "playing " << seeds.size() << " games";
  if (seeds.size() == 1) {
    // auto executor = makeSeerExecutor(4, seeds[0]);
    auto executor = makeRealExecutor(4, seeds[0]);
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
        // auto executor = makeSeerExecutor(4, seeds[gameIdx]);
        auto executor = makeRealExecutor(4, seeds[gameIdx]);
        int score = executor->play(false);
        auto time = t.get();
        LOG_IF(INFO, score <= FLAGS_record_score)
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
    std::cout << "Average score: " << ((double)totalScore / seeds.size())
              << " across " << seeds.size() << " games, with "
              << (100.0 * perfectGames / seeds.size()) << "\% perfect"
              << std::endl;
  }

  return 0;
}
