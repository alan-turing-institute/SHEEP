#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "circuit.hpp"
#include "context-clear.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;

int main(void) {
  typedef std::vector<std::vector<ContextClear<int8_t>::Plaintext>> PtVec;

  Circuit circ = single_binary_gate_circuit(Gate::Compare);

  ContextClear<int8_t> ctx;
  ctx.set_parameter("NumSlots", 1);
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{{1}, {0}}, PtVec{{1}}));
  ctx.set_parameter("NumSlots", 3);
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{{0, 1, -9}, {0, 1, -12}},
                                    PtVec{{0, 0, 1}}));
};
