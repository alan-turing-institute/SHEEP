#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::TFHE;
	typedef std::vector<ContextTFHE<int8_t>::Plaintext> PtVec;

	Circuit circ;
	Wire s = circ.add_input("s");
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
	circ.set_output(out);

	ContextTFHE<int8_t> ctx;
	
	assert(all_equal(ctx.eval_with_plaintexts(circ, {0, 2, -1}), {-1}));
	assert(all_equal(ctx.eval_with_plaintexts(circ, {1, 0, 0}), {0}));
	assert(all_equal(ctx.eval_with_plaintexts(circ, {1, 100, 0}), {100}));
}
