#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextHElib_Fp<bool>::Plaintext> PtVec;

  Circuit circ;
  Wire in = circ.add_input("in");
  Wire out = circ.add_assignment("out", Gate::Negate, in);
  circ.set_output(out);

  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_Fp<bool> ctx;

  std::vector<std::vector<ContextHElib_Fp<bool>::Plaintext>> pt_input = {
      {true, false}};

  std::vector<std::vector<ContextHElib_Fp<bool>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input);

  std::vector<bool> exp_values = {false, true};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << "- (" << std::to_string(pt_input[0][i])
              << ") = " << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
