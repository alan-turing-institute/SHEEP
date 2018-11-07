#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextTFHE<int8_t>::Plaintext> PtVec;

  Circuit circ;
  Wire in = circ.add_input("in");
  Wire out = circ.add_assignment("out", Gate::Negate, in);
  circ.set_output(out);

  ContextTFHE<int8_t> ctx;

  assert(eval_encrypted_check_equal(ctx, circ, PtVec{0}, PtVec{0}));
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{1}, PtVec{-1}));
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{-127}, PtVec{127}));
  // Correct for int8_t (eight-bit two's complement):
  assert(eval_encrypted_check_equal(ctx, circ, PtVec{-128}, PtVec{-128}));
}
