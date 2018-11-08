#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-clear.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextClear<bool>::Plaintext> PtVec;

  Circuit circ;
  Wire s = circ.add_input("s");
  Wire a = circ.add_input("a");
  Wire b = circ.add_input("b");
  Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
  circ.set_output(out);

  ContextClear<bool> ctx;

  assert(all_equal(ctx.eval_with_plaintexts(circ, {0, 1, 0}), {0}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {1, 0, 0}), {0}));
  assert(all_equal(ctx.eval_with_plaintexts(circ, {1, 1, 0}), {1}));
}
