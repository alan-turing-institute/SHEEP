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
  ContextHElib_F2<int8_t> ctx;

  /// first input is select bit
  std::vector<std::vector<int8_t> > pt_input = {
      {1, 1, 0, 0}, {11, 22, 33, 44}, {21, -32, 43, -54}};
  std::vector<std::vector<int8_t> > result =
      ctx.eval_with_plaintexts(circ, pt_input);
  std::vector<int8_t> exp_values = {11, 22, 43, -54};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " ?  "
              << std::to_string(pt_input[1][i]) << " : "
              << std::to_string(pt_input[2][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
