#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {

  using namespace SHEEP;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	std::vector<DurationT> durations;
	ContextHElib_F2<uint8_t> ctx;

	std::vector<std::vector<ContextHElib_F2<uint8_t>::Plaintext>> pt_input = {{15, 255, 0}};

	std::vector<std::vector<ContextHElib_F2<uint8_t>::Plaintext>> result = ctx.eval_with_plaintexts(circ, pt_input, durations);

	std::vector<uint8_t> exp_values = {241, 1, 0};

	for (int i = 0; i < exp_values.size(); i++) {
	  std::cout << "- (" << std::to_string(pt_input[0][i]) << ") = " << std::to_string(result[0][i]) << std::endl;
	  assert(result.front()[i] == exp_values[i]);
	}
}
