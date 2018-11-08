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

  Circuit circ = cr.create_circuit(Gate::Multiply, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_F2<uint8_t> ctx;

  // ContextHElib_F2<uint8_t>::CircuitEvaluator run_circuit;
  // run_circuit = ctx.compile(circ);

  /// test small postitive numbers
  std::vector<uint8_t> inputs = {5, 22};
  std::vector<uint8_t> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 5*22 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == 110);

  /// test result going out of range positive
  inputs = {100, 127};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 100 * 127 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == 156);
}
