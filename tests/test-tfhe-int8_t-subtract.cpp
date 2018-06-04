#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextTFHE<int8_t>::Plaintext> PtVec;

	Circuit circ = single_binary_gate_circuit(Gate::Subtract);

	ContextTFHE<int8_t> ctx;

	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0,0}, PtVec{0}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0,1}, PtVec{-1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{1,0}, PtVec{1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{10,12}, PtVec{-2}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0,-1}, PtVec{1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{-1,-2}, PtVec{1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{10,-12}, PtVec{22}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{127,1}, PtVec{126}));
	// etc...
}
