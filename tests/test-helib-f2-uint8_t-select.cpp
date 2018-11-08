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
  Wire s = circ.add_input("s");
  Wire a = circ.add_input("a");
  Wire b = circ.add_input("b");
  Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
  circ.set_output(out);

  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_F2<uint8_t> ctx;

  /// test choosing first
  std::vector<uint8_t> inputs = {1, 3, 15};
  std::vector<uint8_t> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);
  assert(result.front() == 3);
  std::cout << " select (1, 3, 15) = " << std::to_string(result.front())
            << std::endl;
  /// test choosing second
  inputs = {0, 12, 55};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  assert(result.front() == 55);
  std::cout << " select(0, 12, 55) = " << std::to_string(result.front())
            << std::endl;
}
