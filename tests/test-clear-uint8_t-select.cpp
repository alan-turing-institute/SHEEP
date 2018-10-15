#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-clear.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextClear<uint8_t>::Plaintext> PtVec;

	Circuit circ;
	Wire s = circ.add_input("s");
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
	circ.set_output(out);

	ContextClear<uint8_t> ctx;
	
	assert(all_equal(ctx.eval_with_plaintexts(circ, {{0}, {2}, {111}}), {{111}}));

	assert(all_equal(ctx.eval_with_plaintexts(circ, {{1, 1}, {0, 200}, {0, 2}}), {{0, 200}}));
}
