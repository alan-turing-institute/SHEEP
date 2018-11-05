#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"
#include "circuit-repo.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace SHEEP;

	CircuitRepo cr;

	Circuit circ = cr.create_circuit(Gate::Multiply, 1);
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextTFHE<int8_t> ctx;
	ctx.set_parameter("NumSlots",4);
	std::vector<std::vector<ContextTFHE<int8_t>::Plaintext>> pt_input = {{3, 10, 10, -120}, {15, -12, 127, 0}};

	std::vector<std::vector<ContextTFHE<int8_t>::Plaintext>> result = ctx.eval_with_plaintexts(circ, pt_input, durations);

	std::vector<int8_t> exp_values = {45, -120, -10, 0};

	for (int i = 0; i < exp_values.size(); i++) {
	  std::cout << std::to_string(pt_input[0][i]) << " * " <<  std::to_string(pt_input[1][i]) << " = " << std::to_string(result[0][i]) << std::endl;
	}

	assert(result.front() == exp_values);
}
