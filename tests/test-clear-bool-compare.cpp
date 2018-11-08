#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "circuit.hpp"
#include "context-clear.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;

int main(void) {
  typedef std::vector<ContextClear<bool>::Plaintext> PtVec;

  Circuit circ = single_binary_gate_circuit(Gate::Compare);

  ContextClear<bool> ctx;

  assert(eval_encrypted_check_equal(ctx, circ, PtVec{1, 0}, PtVec{1}));
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{0, 1}, PtVec{0}));

  assert(eval_encrypted_check_equal(ctx, circ, PtVec{0, 0}, PtVec{0}));
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{1, 1}, PtVec{0}));
};
