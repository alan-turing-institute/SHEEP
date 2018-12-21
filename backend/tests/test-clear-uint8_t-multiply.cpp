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

  Circuit circ = cr.create_circuit(Gate::Multiply, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextClear<uint8_t> ctx;

  // test small postitive numbers
  std::vector<std::vector<uint8_t>> inputs = {{5}, {22}};
  std::vector<uint8_t> exp_values = {110};

  std::vector<std::vector<uint8_t>> result =
      ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " * "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }

  /// test result going out of range positive
  inputs = {{100, 0}, {127, 1}};
  exp_values = {156, 0};

  result = ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " * "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
