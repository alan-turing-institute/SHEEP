#include <algorithm>
#include <cassert>
#include "circuit-test-util.hpp"
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextTFHE<bool>::Plaintext> PtVec;

  // Multiply corresponds to 'And' in binary circuit
  Circuit circ = single_binary_gate_circuit(Gate::Multiply);

  ContextTFHE<bool> ctx;

  assert(eval_encrypted_check_equal(ctx, circ, PtVec{true, true}, PtVec{true}));
  assert(
      eval_encrypted_check_equal(ctx, circ, PtVec{false, true}, PtVec{false}));
  assert(
      eval_encrypted_check_equal(ctx, circ, PtVec{true, false}, PtVec{false}));
  assert(
      eval_encrypted_check_equal(ctx, circ, PtVec{false, false}, PtVec{false}));
}
