#include <cassert>
#include <memory>
#include "context-clear.hpp"

#include "circuit-repo.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

using namespace SHEEP;

int main(void) {
  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Subtract, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextClear<int8_t> ctx;

  /// test small postitive numbers
  std::vector<std::vector<int8_t>> inputs = {{22}, {15}};
  std::vector<int8_t> exp_values = {7};

  std::vector<std::vector<int8_t>> result =
      ctx.eval_with_plaintexts(circ, inputs, durations);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " - "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }

  inputs = {{10, -13, 0}, {12, 66, 22}};
  exp_values = {-2, -79, -22};

  result = ctx.eval_with_plaintexts(circ, inputs, durations);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " - "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
