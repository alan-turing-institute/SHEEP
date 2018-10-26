#include <memory>
#include <cassert>
#include "context-clear.hpp"

#include "circuit-repo.hpp"

using namespace SHEEP;

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;


  Circuit C;
  Wire in0 = C.add_input("in0");
  Wire cin0 = C.add_const_input("cin0");
  Wire out0 = C.add_assignment("out0", Gate::AddConstant, in0, cin0);

  C.set_output(out0);

  std::cout << C;
  std::vector<DurationT> durations;

  // The type of the wires in circ are unsigned 8-bit numbers
  ContextClear<uint8_t> ctx;

  // inputs is vector of vectors
  std::vector<std::vector<uint8_t>> inputs = {{117, 2, 3, 4}};
  // const_inputs is vector (same across slots)
  std::vector<uint8_t> const_inputs = {6};
  std::vector<uint8_t> exp_values = {123, 8, 9, 10};

  std::vector<std::vector<uint8_t>> result = ctx.eval_with_plaintexts(C, inputs, const_inputs, durations);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(inputs[0][i]) << " + " <<  std::to_string(const_inputs[0]) << " = " << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
