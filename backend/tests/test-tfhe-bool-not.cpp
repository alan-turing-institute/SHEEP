#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextTFHE<bool>::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	std::cout << circ;
	std::vector<DurationT> durations;
	ContextTFHE<bool> ctx;
	ctx.set_parameter("NumSlots",2);
	std::vector<std::vector<ContextTFHE<bool>::Plaintext>> pt_input = {{true, false}};
	std::vector<std::vector<ContextTFHE<bool>::Plaintext>> result = ctx.eval_with_plaintexts(circ, pt_input, durations);

	std::vector<bool> exp_values = {false, true};

	for (int i = 0; i < exp_values.size(); i++) {
	  std::cout << "- (" << std::to_string(pt_input[0][i]) << ") = " << std::to_string(result[0][i]) << std::endl;
	}

  assert(result.front() == exp_values);
}
