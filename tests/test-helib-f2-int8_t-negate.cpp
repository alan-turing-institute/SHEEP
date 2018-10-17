#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"
#include "circuit-repo.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {

  using namespace SHEEP;
  
  CircuitRepo cr;
	Circuit circ = cr.create_circuit(Gate::Negate, 1);
	std::cout << circ;
  std::vector<DurationT> durations;
	ContextHElib_F2<int8_t> ctx;

	std::vector<std::vector<ContextHElib_F2<int8_t>::Plaintext>> pt_input = {{3, 10, -120}};

	std::vector<std::vector<ContextHElib_F2<int8_t>::Plaintext>> result = ctx.eval_with_plaintexts(circ, pt_input, durations);

	std::vector<int8_t> exp_values = {-3, -10, 120};

	for (int i = 0; i < exp_values.size(); i++) {
    std::cout << "- (" << std::to_string(pt_input[0][i]) << ") = " << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
