#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextHElib_F2<bool>::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);
	std::vector<DurationT> durations;
	std::cout<<circ;
	
	ContextHElib_F2<bool> ctx;

///  positive to negative
        std::vector<bool> inputs = {true};
        std::vector<bool> result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(true) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == false);
	///  negative to positive
	inputs = {false};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(false) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == true);

}
