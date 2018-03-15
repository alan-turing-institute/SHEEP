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

	Circuit circ = cr.create_circuit(Gate::Subtract, 1);
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextHElib_Fp<uint8_t> ctx;

	/// test small postitive numbers
	std::vector<uint8_t> inputs = {22, 15};
	std::vector<uint8_t> result = ctx.eval_with_plaintexts(circ, inputs, durations);
	assert(result.front() == 7);
	std::cout<<" 22 - 15 = "<<std::to_string(result.front())<<std::endl;	
	/// test small 'negative' numbers (should give 2^BITWIDTH - x)
	inputs = {10, 12};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	assert(result.front() == 254);
	std::cout<<" 10 - 12 = "<<std::to_string(result.front())<<std::endl;
	/// give zero?
	inputs = {255, 255};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" 255 - 255 = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == 0);
	/// max negative
	inputs = {0, 255};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" 0 - 255 = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == 1);

}
