#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"


using namespace SHEEP;

int main(void) {
	ContextClear<uint8_t> ctx;
	assert(ctx.RippleCarryAdd(10, 12) == 22);
	std::cout << std::to_string(ctx.RippleCarryAdd(10, 122)) << "\n";
	assert(ctx.RippleCarryAdd(10, 122) == 132);
}
