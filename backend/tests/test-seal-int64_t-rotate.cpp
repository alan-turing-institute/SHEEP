#include <cassert>
#include <memory>
#include "context-seal.hpp"

#include "circuit-repo.hpp"

using namespace SHEEP;

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  Circuit C;
  Wire in0 = C.add_input("in0");
  Wire cin0 = C.add_const_input("cin0");
  Wire out0 = C.add_assignment("out0", Gate::Rotate, in0, cin0);

  C.set_output(out0);

  std::cout << C;
  std::vector<DurationT> durations;

  // The type of the wires in circ are unsigned bools
  ContextSeal<int64_t> ctx;

  std::cout << "Number of slots provided by SEAL: " << ctx.get_num_slots()
            << "\n";

  // inputs is vector of vectors
  int num_inputs = 100;
  std::vector<std::vector<int64_t> > inputs(1);
  inputs[0].resize(num_inputs);

  std::vector<int64_t> exp_values(num_inputs);
  for (int i = 0; i < num_inputs; i++) {
    inputs[0][i] = i;
    exp_values[i] = i - 1;
  }
  exp_values[0] = num_inputs - 1;

  // const_inputs is vector (not vector<vector>), same across slots
  std::vector<long> const_inputs = {1};

  std::vector<std::vector<int64_t> > result =
      ctx.eval_with_plaintexts(C, inputs, const_inputs);

  std::cout << "Original vector: ";
  for (int i = 0; i < num_inputs; i++) {
    std::cout << std::to_string(inputs[0][i]) << " ";
  }
  std::cout << "\nRotated vector:  ";
  for (int i = 0; i < num_inputs; i++) {
    std::cout << std::to_string(result[0][i]) << " ";
  }
  for (int i = 0; i < num_inputs; i++) {
    assert(result[0][i] == exp_values[i]);
  }

  std::cout << std::endl;

}
