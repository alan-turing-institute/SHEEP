#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-clear.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextClear<uint8_t> ctx;

  // ContextClear<uint8_t>::CircuitEvaluator run_circuit;
  // run_circuit = ctx.compile(circ);

  /// test small postitive numbers
  std::vector<uint8_t> inputs = {5, 22};
  std::vector<uint8_t> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 5+22 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == 27);

  /// test result going out of range positive
  inputs = {200, 127};
  result = ctx.eval_with_plaintexts(circ, inputs, durations);
  std::cout << " 200 + 127 = " << std::to_string(result.front()) << std::endl;
  assert(result.front() == 71);
}
