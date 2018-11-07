#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextTFHE<int8_t>::Plaintext> PtVec;

  Circuit circ = single_binary_gate_circuit(Gate::Multiply);

  ContextTFHE<int8_t> ctx;

  assert(all_equal(ctx.eval_with_plaintexts(circ, {0, 0}), {0}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {0, 1}), {0}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {1, 0}), {0}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {1, 1}), {1}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {17, -7}), {-119}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {-7, 17}), {-119}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {2, -127}), {2}));

  // assert(all_equal(ctx.eval_with_plaintexts(circ, {2,1}), {2}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {2,2}), {4}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {2,3}), {6}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {-1,0}), {0}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {-1,1}), {-1}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {-1,-1}), {1}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {-127,-127}), {1}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {2,127}), {-2}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {2,-127}), {2}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {2,64}), {-128}));
  // assert(all_equal(ctx.eval_with_plaintexts(circ, {-128,-128}), {0}));
}
