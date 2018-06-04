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
	
	ContextHElib_Fp<uint8_t> ctx;
	std::vector<DurationT> durations;
	
///  positive to negative
        std::vector<uint8_t> inputs = {15};
        std::vector<uint8_t> result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(15) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == 241);
	///  max 
	inputs = {255};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(255) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == 1);
	/// zero
	inputs = {0};
	result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" negate(0) = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == 0);
	       

}
