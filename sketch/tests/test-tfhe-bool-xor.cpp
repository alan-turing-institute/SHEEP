#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::TFHE;
	typedef std::vector<ContextTFHE::Plaintext> PtVec;

	// Both Add and Subract correspond to 'Xor' in binary circuit
	Circuit circ_add = single_binary_gate_circuit(Gate::Add);
	Circuit circ_sub = single_binary_gate_circuit(Gate::Subtract);

	ContextTFHE ctx;
	
	assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{true, true}, PtVec{false}));
	assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{false, true}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{true, false}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{false, false}, PtVec{false}));
	
	assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{true, true}, PtVec{false}));
	assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{false, true}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{true, false}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{false, false}, PtVec{false}));
}
