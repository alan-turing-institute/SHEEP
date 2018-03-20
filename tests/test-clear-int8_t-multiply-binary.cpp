#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::Clear;
	typedef std::vector<ContextClear<int8_t>::Plaintext> PtVec;

	ContextClear<int8_t> ctx;
	assert(ctx.MultiplyBinary(0,0) == 0);
	assert(ctx.MultiplyBinary(0,1) == 0);
	assert(ctx.MultiplyBinary(1,0) == 0);
	assert(ctx.MultiplyBinary(1,1) == 1);
	assert(ctx.MultiplyBinary(17, -7) == -119);
	assert(ctx.MultiplyBinary(-7, 17) == -119);
	assert(ctx.MultiplyBinary(2,-127) == 2);
	assert(ctx.MultiplyBinary(2,1) == 2);
	assert(ctx.MultiplyBinary(2,2) == 4);
	assert(ctx.MultiplyBinary(2,3) == 6);
	assert(ctx.MultiplyBinary(-1,0) == 0);
	assert(ctx.MultiplyBinary(-1,1) == -1);
	assert(ctx.MultiplyBinary(-1,-1) == 1);
	assert(ctx.MultiplyBinary(-127,-127) == 1);
	assert(ctx.MultiplyBinary(2,127) == -2);
	assert(ctx.MultiplyBinary(2,-127) == 2);
	assert(ctx.MultiplyBinary(2,64) == -128);
	assert(ctx.MultiplyBinary(-128,-128) == 0);
}
