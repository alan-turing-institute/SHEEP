#include <cstdint>
#include <cassert>
#include "context-clear.hpp"
#include "circuit.hpp"
#include "circuit-test-util.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;

int main(void) {
        typedef std::vector<ContextClear<int8_t>::Plaintext> PtVec;
	
	Circuit circ = single_binary_gate_circuit(Gate::Compare);
  
	ContextClear<int8_t> ctx;
	
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{1, 0}, PtVec{1}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0, -1}, PtVec{1}));

	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0, 0}, PtVec{0}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{1, 1}, PtVec{0}));

	assert(eval_encrypted_check_equal(ctx, circ, PtVec{0, 1}, PtVec{0}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{-1, 1}, PtVec{0}));
	assert(eval_encrypted_check_equal(ctx, circ, PtVec{-1, 2}, PtVec{0}));
};
