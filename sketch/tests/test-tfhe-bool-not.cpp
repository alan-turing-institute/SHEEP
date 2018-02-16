#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::TFHE;
	typedef std::vector<ContextTFHE::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	ContextTFHE ctx;

	assert(eval_encrypted_check_equal(ctx, circ, PtVec{true}, PtVec{false}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{false}, PtVec{true}));
}
