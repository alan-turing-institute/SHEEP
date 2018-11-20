#include <algorithm>
#include <cassert>
#include <cstdint>
#include "context-clear.hpp"

using namespace SHEEP;

int main(void) {
  ContextClear<bool> ctx;

  std::vector<bool> a, b, c;

  a = {1, 1, 0, 0};
  b = {1, 0, 1, 0};
  c = {0, 1, 1, 0};

  assert(ctx.RippleCarryAdd(a, b) == c);

  // assert(ctx.RippleCarryAdd(1, 1) == {{0}});

  // // std::cout << std::to_string(ctx.RippleCarryAdd(1, 0)) << "\n";
  // // assert(ctx.RippleCarryAdd(1, 0) == 1);
}
