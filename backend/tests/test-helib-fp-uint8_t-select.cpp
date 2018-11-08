#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  Circuit circ;
  Wire s = circ.add_input("s");
  Wire a = circ.add_input("a");
  Wire b = circ.add_input("b");
  Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
  circ.set_output(out);

  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_Fp<int8_t> ctx;

  std::vector<std::vector<ContextHElib_Fp<int8_t>::Plaintext>> pt_input = {
      {1, 0}, {3, 12}, {15, 55}};

  std::vector<std::vector<ContextHElib_Fp<int8_t>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input, durations);

  std::vector<int8_t> exp_values = {3, 55};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << "select: " << std::to_string(pt_input[0][i]) << ", "
              << std::to_string(pt_input[1][i]) << ", "
              << std::to_string(pt_input[2][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
