#include <memory>
#include <cassert>
#include "context-clear.hpp"

#include "circuit-repo.hpp"

using namespace SHEEP;

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;
  
  //// instantiate the Circuit Repository
  CircuitRepo cr;

  // circ contains an Add gate with two inputs and one output
  Circuit circ = cr.create_circuit(Gate::Add, 3);

  std::cout << circ;
  std::vector<DurationT> durations;
  
  // The type of the wires in circ are signed 8-bit numbers
  ContextClear<int8_t> ctx;

  // test small postitive numbers
  std::vector<std::vector<int8_t>> inputs = {{1, -9}, {2, 8}, {3, 1}, {4, 0}};
  std::vector<int8_t> exp_values = {10, 0};

  std::vector<std::vector<int8_t>> result = ctx.eval_with_plaintexts(circ, inputs, durations);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " + " <<  std::to_string(inputs[1][i]) << " + " << std::to_string(inputs[2][i]) << " + " <<  std::to_string(inputs[3][i]) << " = " << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
