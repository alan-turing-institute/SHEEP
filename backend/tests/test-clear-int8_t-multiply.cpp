#include <cassert>
#include <memory>
#include "context-clear.hpp"

#include "circuit-repo.hpp"

using namespace SHEEP;
typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  // instantiate the Circuit Repository
  CircuitRepo cr;

  // build a circuit with a specified depth of a specified gate
  Circuit circ = cr.create_circuit(Gate::Multiply, 3);
  std::cout << circ;

  ContextClear<int8_t> ctx;
  std::vector<DurationT> durations;

  // test small postitive numbers
  std::vector<std::vector<int8_t>> inputs = {{1}, {-2}, {3}, {4}};
  std::vector<int8_t> exp_values = {-24};

  std::vector<std::vector<int8_t>> result =
      ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " * "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }

  // Depth 1
  circ = cr.create_circuit(Gate::Multiply, 1);
  std::cout << circ;

  inputs = {{100, 0}, {1, 1}};
  exp_values = {100, 0};

  result = ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " * "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
