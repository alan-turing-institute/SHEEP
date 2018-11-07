#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  Circuit circ;
  Wire a = circ.add_input("a");
  Wire b = circ.add_input("b");
  Wire out = circ.add_assignment("out", Gate::Compare, a, b);
  circ.set_output(out);

  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_Fp<uint8_t> ctx;

  /// test first bigger
  std::vector<uint8_t> inputs = {6, 3};
  std::vector<uint8_t> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " compare (6,3 ) = " << std::to_string(result.front())
            << std::endl;
  assert(result.front() == 1);

  /// test equal
  inputs = {12, 12};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " compare (12, 12) = " << std::to_string(result.front())
            << std::endl;
  assert(result.front() == 0);
  /// test second bigger - both positive
  inputs = {1, 112};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " compare (1, 112) = " << std::to_string(result.front())
            << std::endl;
  assert(result.front() == 0);
}
