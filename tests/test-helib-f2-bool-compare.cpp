#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace SHEEP;

	Circuit circ;
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Compare, a, b);
	circ.set_output(out);

	
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextHElib_F2<bool> ctx;

	/// test first bigger
	std::vector<bool> inputs = {1, 0};
	std::vector<bool> result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" compare (1,0 ) = "<<std::to_string(result.front())<<std::endl;	
	assert(result.front() == 1);

	/// test equal
	inputs = {1, 1};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" compare (1, 1) = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == 0);
	inputs = {0, 0};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" compare (0, 0) = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == 0);
	
	/// test second bigger 
	inputs = {0, 1};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
	std::cout<<" compare (0, 1) = "<<std::to_string(result.front())<<std::endl;
	assert(result.front() == 0);

}
