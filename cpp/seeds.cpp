#include "seeds.h"

#include <fstream>

#include "common/time/ChronoFlags.h"

DEFINE_int32(ngames, 0, "");
DEFINE_int32(seed, 0, "");
DEFINE_string(seed_file, "", "Read seeds from this file");

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
