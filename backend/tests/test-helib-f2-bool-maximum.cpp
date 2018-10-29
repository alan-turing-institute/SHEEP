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
	Wire out = circ.add_assignment("out", Gate::Maximum, a, b);
	circ.set_output(out);


	std::cout << circ;
	std::vector<DurationT> durations;
	ContextHElib_F2<bool> ctx;

	///
	std::vector< std::vector<bool> > inputs = {{1,1,0,0},{0,1,0,1}};
	std::vector<bool> expected_result = {1,1,1,0};
	std::vector< std::vector<bool> > result = ctx.eval_with_plaintexts(circ, inputs, durations);
	for (int i=0; i< expected_result.size(); i++) {
	  std::cout<<" compare ("<<inputs[0][i]<<","<<inputs[1][i]<<") = "<<std::to_string(result.front()[i])<<std::endl;
	  //	  assert(result.front()[i] == expected_result[i]);
	}


}
