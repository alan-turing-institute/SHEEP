#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace SHEEP;
	typedef std::vector<ContextHElib_Fp<int8_t>::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	std::cout<<circ;
	std::vector<DurationT> durations;
	ContextHElib_Fp<int8_t> ctx;

///  positive to negative
        std::vector<int8_t> inputs = {15};
        std::vector<int8_t> result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(15) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == -15);
	///  negative to positive
	inputs = {-15};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(-15) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == 15);
	/// max  negative
	inputs = {-128};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(-128) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == -128);
	
	       

}
