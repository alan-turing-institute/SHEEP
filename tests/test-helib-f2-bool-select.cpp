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
	Wire s = circ.add_input("s");
	Wire a = circ.add_input("a");
	Wire b = circ.add_input("b");
	Wire out = circ.add_assignment("out", Gate::Select, s, a, b);
	circ.set_output(out);


	std::cout << circ;
	std::vector<DurationT> durations;
	ContextHElib_F2<bool> ctx;


	/// first input is select bit
	std::vector<std::vector<bool> > pt_input = {{1, 1, 0, 0},{1, 0, 1, 0}, {0, 1, 0, 1}};
	std::vector<std::vector<bool> > result = ctx.eval_with_plaintexts(circ, pt_input, durations);
	std::vector<bool> exp_values = {1, 0, 0, 1};

	for (int i = 0; i < exp_values.size(); i++) {
	  std::cout << std::to_string(pt_input[0][i]) << " ?  " <<  std::to_string(pt_input[1][i]) << " : " << std::to_string(pt_input[2][i]) <<" = "<< std::to_string(result[0][i]) << std::endl;
	  assert(result.front()[i] == exp_values[i]);
	}

}
