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

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_F2<int8_t> ctx;

  /// test small postitive numbers
  std::vector<int8_t> inputs = {15, 22};
  std::vector<int8_t> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 15+22 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == 37);

  /// test small negative numbers
  inputs = {10, -12};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 10 - 12 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == -2);

  /// test result going out of range positive
  inputs = {100, 127};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 100 + 127 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == -29);

  /// test result going out of range negative
  inputs = {-120, -124};
  std::cout << " -120 - 124 = " << std::to_string(result.front()) << std::endl;
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  assert(result.front() == 12);
}
