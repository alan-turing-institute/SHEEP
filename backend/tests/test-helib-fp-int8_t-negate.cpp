#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  Circuit circ;
  Wire in = circ.add_input("in");
  Wire out = circ.add_assignment("out", Gate::Negate, in);
  circ.set_output(out);

  std::vector<DurationT> durations;
  ContextHElib_Fp<int8_t> ctx;

  std::vector<std::vector<ContextHElib_Fp<int8_t>::Plaintext>> pt_input = {
      {15, -15, -128}};

  std::vector<std::vector<ContextHElib_Fp<int8_t>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input, durations);

  std::vector<int8_t> exp_values = {-15, 15, -128};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << "- (" << std::to_string(pt_input[0][i])
              << ") = " << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
