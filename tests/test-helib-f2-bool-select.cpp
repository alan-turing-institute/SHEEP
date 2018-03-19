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

	Circuit circ;
	Wire s = circ.add_input("s");
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
	circ.set_output(out);

	
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextHElib_F2<bool> ctx;

	/// test choosing first
	std::vector<bool> inputs = {1, 0, 1};
	std::vector<bool> result = ctx.eval_with_plaintexts(circ, inputs, durations);
	assert(result.front() == 0);
	std::cout<<" select (1, 0, 1) = "<<std::to_string(result.front())<<std::endl;	
	/// test choosing second
	inputs = {0, 0, 1};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	assert(result.front() == 1);
	std::cout<<" select(0, 0, 1) = "<<std::to_string(result.front())<<std::endl;

}
