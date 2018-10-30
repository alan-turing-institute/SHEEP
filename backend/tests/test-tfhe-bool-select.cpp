#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "circuit-test-util.hpp"

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
  ContextTFHE<bool> ctx;

  std::vector<std::vector<ContextTFHE<bool>::Plaintext>> pt_input = {{1, 0}, {0, 0}, {1, 1}};

	std::vector<std::vector<ContextTFHE<bool>::Plaintext>> result = ctx.eval_with_plaintexts(circ, pt_input, durations);

	std::vector<bool> exp_values = {0, 1};

	for (int i = 0; i < exp_values.size(); i++) {
    std::cout << "select: " << std::to_string(pt_input[0][i]) << ", " << std::to_string(pt_input[1][i]) << ", " << std::to_string(pt_input[2][i]) << " = " << std::to_string(result[0][i]) << std::endl;
  }

  assert(result.front() == exp_values);
}