#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace Sheep::HElib;

    //// instantiate the Circuit Repository
	CircuitRepo cr;

	Circuit circ = cr.create_circuit(Gate::Multiply, 1);
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextHElib_F2<int8_t> ctx;

	/// test small postitive numbers
	std::list<int8_t> inputs = {3, 15};
	std::list<int8_t> result = ctx.eval_with_plaintexts(circ, inputs, durations);
	assert(result.front() == 45);
	std::cout<<" 3* 15 = "<<std::to_string(result.front())<<std::endl;	
	/// test small negative numbers
	inputs = {10, -12};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	assert(result.front() == -120);
	std::cout<<" 10 * -12 = "<<std::to_string(result.front())<<std::endl;
	/// test result going out of range positive
	inputs = {10, 127};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" 10 * 127 = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == -10);
	/// test result going out of range negative
	inputs = {-120, 124};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" -120 * 124 = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == -32);

}
