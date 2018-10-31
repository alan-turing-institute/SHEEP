#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextTFHE<uint8_t>::Plaintext> PtVec;

	Circuit circ = single_binary_gate_circuit(Gate::Add);

	ContextTFHE<uint8_t> ctx;

	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0,0}, PtVec{0}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0,1}, PtVec{1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{1,0}, PtVec{1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{10,12}, PtVec{22}));
	// etc...
}
