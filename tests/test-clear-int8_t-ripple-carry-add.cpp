#include <algorithm>
#include <cassert>
#include <cstdint>
#include "context-clear.hpp"

using namespace SHEEP;

int main(void) {
  ContextClear<int8_t> ctx;
  assert(ctx.RippleCarryAdd(10, 12) == 22);
  std::cout << std::to_string(ctx.RippleCarryAdd(10, -12)) << "\n";
  assert(ctx.RippleCarryAdd(10, -12) == -2);
}
