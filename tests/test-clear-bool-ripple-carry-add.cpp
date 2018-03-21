#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"


using namespace SHEEP;

int main(void) {
	ContextClear<bool> ctx;
	assert(ctx.RippleCarryAdd(1, 1) == 0);
	std::cout << std::to_string(ctx.RippleCarryAdd(1, 0)) << "\n";
	assert(ctx.RippleCarryAdd(1, 0) == 1);
}
