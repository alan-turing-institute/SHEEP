#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"


using namespace SHEEP;

int main(void) {
	ContextClear<int8_t> ctx;
	
	std::vector<int8_t> a, b, c;

	a = {10, 10};
	b = {12, -3};
	c = {22, 7};

	std::cout << std::to_string(a[0]) << " + " << std::to_string(b[0]) << " = " << std::to_string(ctx.RippleCarryAdd(a, b)[0]) << "\n";

	assert(ctx.RippleCarryAdd(a, b) == c);
}
