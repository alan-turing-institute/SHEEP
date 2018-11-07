#include <algorithm>
#include <cassert>
#include "circuit-test-util.hpp"
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextTFHE<bool>::Plaintext> PtVec;

  // Both Add and Subract correspond to 'Xor' in binary circuit
  Circuit circ_add = single_binary_gate_circuit(Gate::Add);
  Circuit circ_sub = single_binary_gate_circuit(Gate::Subtract);

  ContextTFHE<bool> ctx;

  assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{true, true},
                                    PtVec{false}));
  assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{false, true},
                                    PtVec{true}));
  assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{true, false},
                                    PtVec{true}));
  assert(eval_encrypted_check_equal(ctx, circ_add, PtVec{false, false},
                                    PtVec{false}));

  assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{true, true},
                                    PtVec{false}));
  assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{false, true},
                                    PtVec{true}));
  assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{true, false},
                                    PtVec{true}));
  assert(eval_encrypted_check_equal(ctx, circ_sub, PtVec{false, false},
                                    PtVec{false}));
}
