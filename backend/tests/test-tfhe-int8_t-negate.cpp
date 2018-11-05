#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
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
	ContextTFHE<int8_t> ctx;
	ctx.set_parameter("NumSlots",4);
	std::vector<std::vector<ContextTFHE<int8_t>::Plaintext>> pt_input = {{15, -15, -128, 0}};

	std::vector<std::vector<ContextTFHE<int8_t>::Plaintext>> result = ctx.eval_with_plaintexts(circ, pt_input, durations);

	std::vector<int8_t> exp_values = {-15, 15, -128, 0};

	for (int i = 0; i < exp_values.size(); i++) {
	  std::cout << "- (" << std::to_string(pt_input[0][i]) << ") = " << std::to_string(result[0][i]) << std::endl;
	}

	assert(result.front() == exp_values);
}
