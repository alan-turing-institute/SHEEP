#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::TFHE;
	typedef std::vector<ContextTFHE<bool>::Plaintext> PtVec;

	Circuit circ;
	Wire s = circ.add_input("s");
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
	circ.set_output(out);

	ContextTFHE<bool> ctx;
	
	assert(all_equal(eval_with_plaintexts(ctx, circ, PtVec{0, 0, 1}), PtVec{1}));
	assert(all_equal(eval_with_plaintexts(ctx, circ, PtVec{1, 0, 1}), PtVec{0}));
}
