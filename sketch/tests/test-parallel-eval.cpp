#include <cstdint>
#include <cassert>
#include "circuit.hpp"
#include "circuit-test-util.hpp"
#include "simple-circuits.hpp"
#include "context-tfhe.hpp"

using namespace Sheep::TFHE;

int main(void) {

	typedef std::vector<ContextTFHE<int8_t>::Plaintext> PtVec;
	
	Circuit C;
	Wire a = C.add_input("a");
	Wire b = C.add_input("b");
	Wire c = C.add_input("c");

	Wire a1 = C.add_assignment("a1", Gate::Negate, a);
	Wire a2 = C.add_assignment("a2", Gate::Negate, a1);

	Wire b1 = C.add_assignment("b1", Gate::Negate, b);
	Wire b2 = C.add_assignment("b2", Gate::Negate, b1);

	Wire c1 = C.add_assignment("c1", Gate::Negate, c);
	Wire c2 = C.add_assignment("c2", Gate::Negate, c1);
	
	Wire d = C.add_assignment("d", Gate::Add, a2, b2);
	Wire e = C.add_assignment("e", Gate::Add, b2, c2);

	Wire f = C.add_assignment("f", Gate::Add, d, e);
	
	C.set_output(f);
	
	ContextTFHE<int8_t> ctx;

	std::chrono::duration<double, std::micro> duration1, duration2;
	assert(all_equal(eval_with_plaintexts(ctx, C, PtVec{1,2,3}, duration1), PtVec{8}));
	std::cout << duration1.count() << std::endl;
	assert(all_equal(eval_with_plaintexts(ctx, C, PtVec{1,2,3}, duration2, true), PtVec{8}));
	std::cout << duration2.count() << std::endl;
}
