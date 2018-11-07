#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-clear.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextClear<int8_t>::Plaintext> PtVec;

  Circuit circ;
  Wire s = circ.add_input("s");
  Wire a = circ.add_input("a");
  Wire b = circ.add_input("b");
  Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
  circ.set_output(out);

  ContextClear<int8_t> ctx;
  ctx.set_parameter("NumSlots", 1);
  assert(all_equal(ctx.eval_with_plaintexts(circ, {{0}, {2}, {111}}), {{111}}));
  ctx.set_parameter("NumSlots", 2);
  assert(all_equal(ctx.eval_with_plaintexts(circ, {{1, 1}, {0, 15}, {0, 2}}),
                   {{0, 15}}));
}
