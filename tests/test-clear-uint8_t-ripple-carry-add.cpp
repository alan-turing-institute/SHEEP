#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"


using namespace SHEEP;

int main(void) {
	ContextClear<uint8_t> ctx;
	
	std::vector<uint8_t> a, b, c;

	a = {10, 10};
	b = {12, 122};
	c = {22, 132};

	std::cout << std::to_string(a[0]) << " + " << std::to_string(b[0]) << " = " << std::to_string(ctx.RippleCarryAdd(a, b)[0]) << "\n";

	assert(ctx.RippleCarryAdd(a, b) == c);
	
}
