#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextTFHE<bool>::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	ContextTFHE<bool> ctx;

	assert(eval_encrypted_check_equal(ctx, circ, PtVec{true}, PtVec{false}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{false}, PtVec{true}));
}
