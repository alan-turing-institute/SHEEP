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

  // circ contains an Add gate with two inputs and one output
  Circuit circ = cr.create_circuit(Gate::Multiply, 1);
  std::cout << circ;
  std::vector<DurationT> durations;

  ContextClear<double> ctx;

  // test small postitive numbers
  std::vector<std::vector<double>> inputs = {{1.5}, {22.2}};
  std::vector<double> exp_values = {33.3};

  std::vector<std::vector<double>> result =
      ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " + "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(abs(result.front()[i] - exp_values[i])<0.1);
  }

  // test positive number arrays
  inputs = {{41.1, 5.0, 100.0}, {0.1, 4.2, 111.0}};
  exp_values = {4.11, 21.0, 11100.0};

  result = ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " * "
              << std::to_string(inputs[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(abs(result.front()[i] - exp_values[i])<0.1);
  }
}
