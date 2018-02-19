#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::TFHE;
	typedef std::vector<ContextTFHE<bool>::Plaintext> PtVec;

	// Maximum corresponds to 'Or' in binary circuit
	Circuit circ = single_binary_gate_circuit(Gate::Maximum);

	ContextTFHE<bool> ctx;
	
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{true, true}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{false, true}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{true, false}, PtVec{true}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{false, false}, PtVec{false}));
}
