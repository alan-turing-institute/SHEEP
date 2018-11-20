#include <algorithm>
#include <cassert>
#include "circuit-test-util.hpp"
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  // Multiply corresponds to 'And' in binary circuit
  Circuit circ = single_binary_gate_circuit(Gate::Subtract);
  std::cout << "Circ: " << circ << std::endl;
  ContextTFHE<bool> ctx;
  ctx.set_parameter("NumSlots", 4);
  std::vector<DurationT> durations;

  std::vector<std::vector<ContextTFHE<bool>::Plaintext>> pt_input = {
      {1, 0, 1, 0}, {1, 1, 0, 0}};

  std::vector<std::vector<ContextTFHE<bool>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input, durations);

  std::vector<bool> exp_values = {0, 1, 1, 0};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " + "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
  }

  assert(result.front() == exp_values);
}
