#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-seal.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace SHEEP;

    //// instantiate the Circuit Repository
	CircuitRepo cr;

	Circuit circ = cr.create_circuit(Gate::Multiply, 1);
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextSeal<int8_t> ctx;

	/// test small postitive numbers
	std::vector<std::vector<int8_t> > inputs = {{3}, {15}};
	std::vector<std::vector<int8_t> > result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" 3* 15 = "<<std::to_string(result.front()[0])<<std::endl;	
	assert(result.front()[0] == 45);
	/// test small negative numbers
	inputs = {{10}, {-12}};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" 10 * -12 = "<<std::to_string(result.front()[0])<<std::endl;
	assert(result.front()[0] == -120);

	/// test result going out of range positive
	inputs = {{10}, {127}};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" 10 * 127 = "<<std::to_string(result.front()[0])<<std::endl;
	assert(result.front()[0] == -10);
	/// test result going out of range negative
	inputs = {{-120}, {124}};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" -120 * 124 = "<<std::to_string(result.front()[0])<<std::endl;
	assert(result.front()[0] == -32);

}
