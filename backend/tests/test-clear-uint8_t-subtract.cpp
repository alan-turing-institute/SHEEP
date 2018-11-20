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

  Circuit circ = cr.create_circuit(Gate::Subtract, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextClear<uint8_t> ctx;

  /// test small postitive numbers
  std::vector<std::vector<uint8_t>> inputs = {{22}, {15}};
  std::vector<uint8_t> exp_values = {7};

  std::vector<std::vector<uint8_t>> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " - "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }

  /// test small 'negative' numbers (should give 2^BITWIDTH - x)
  /// give zero?
  /// max negative
  inputs = {{10, 255, 0}, {12, 255, 255}};
  exp_values = {254, 0, 1};

  result = ctx.eval_with_plaintexts(circ, inputs, durations);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " - "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
