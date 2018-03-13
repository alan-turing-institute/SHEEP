#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::Clear;
	typedef std::vector<ContextClear<int8_t>::Plaintext> PtVec;

	Circuit circ;
	Wire s = circ.add_input("s");
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
	circ.set_output(out);

	ContextClear<int8_t> ctx;
	
	assert(all_equal(eval_with_plaintexts(ctx, circ, PtVec{0, 2, -1}), PtVec{-1}));
	assert(all_equal(eval_with_plaintexts(ctx, circ, PtVec{1, 0, 0}), PtVec{0}));
	assert(all_equal(eval_with_plaintexts(ctx, circ, PtVec{1, 100, 0}), PtVec{100}));
}
