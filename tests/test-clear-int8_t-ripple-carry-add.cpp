#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"


using namespace Sheep::Clear;

int main(void) {
	ContextClear<int8_t> ctx;
	assert(ctx.RippleCarryAdd(10, 12) == 22);
	std::cout << std::to_string(ctx.RippleCarryAdd(10, -12)) << "\n";
	assert(ctx.RippleCarryAdd(10, -12) == -2);
}
