#include <cassert>

#include "circuit-repo.hpp"
#include "circuit-util.hpp"
#include "circuit.hpp"

#include "all_equal.hpp"
#include "context-clear.hpp"

int main(void) {
  CircuitRepo cr;
  NameGenerator names;

  Circuit C = cr.get_circuit_by_name("TestCircuit3");
  std::cout << C << std::endl;

  Circuit C_copy = copy(C, names);
  std::cout << C_copy << std::endl;

  // another copy using the same name generator object should not be
  // equal to the first copy:
  assert(C_copy != copy(C, names));

  // using a new name generator should produce an identical copy
  // to the first:
  NameGenerator fresh_names;
  assert(C_copy == copy(C, fresh_names));

  // The copy should represent the same circuit.  Compare result
  // of evaluting original and copy

  using namespace SHEEP;
  typedef std::vector<std::vector<ContextClear<int32_t>::Plaintext> > PtVec;
  ContextClear<int32_t> ctx;
  PtVec inputs{{1}, {2}, {3}, {4}, {5}};
  PtVec result_C(ctx.eval_with_plaintexts(C, inputs));
  assert(all_equal(result_C, ctx.eval_with_plaintexts(C_copy, inputs)));
  assert(!all_equal(
      result_C, ctx.eval_with_plaintexts(C_copy, {{1}, {1}, {1}, {1}, {1}})));
}
