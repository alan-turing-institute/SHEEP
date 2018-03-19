#include <cstdint>
#include <cassert>
#include "circuit.hpp"
#include "circuit-test-util.hpp"
#include "simple-circuits.hpp"

// we attempt the evaluation with several contexts

#include "context-clear.hpp"

#if HAVE_TFHE
#include "context-tfhe.hpp"
#endif

#if HAVE_HElib
#include "context-helib.hpp"
#endif

// Since we need a value for USE_TBB below
#ifdef HAVE_TBB
#define HAVE_TBB 1
#else
#define HAVE_TBB 0
#endif

#ifdef HAVE_TFHE
using namespace Sheep::TFHE;
#endif

#ifdef HAVE_TFHE
using namespace Sheep::HElib;
#endif

template <typename ContextT>
void test(const Circuit& C, std::vector<typename ContextT::Plaintext> inputs,
	  std::vector<typename ContextT::Plaintext> outputs)
{
	ContextT ctx;
	
	std::vector<std::chrono::duration<double, std::micro> > duration1, duration2;
	
	assert(all_equal(ctx.eval_with_plaintexts(C, inputs, duration1), outputs));
	std::cout << "evaluating in serial took " << duration1[1].count()
		  << " microseconds" << std::endl;
	
#if ! HAVE_TBB
	try {
#endif
		assert(all_equal(ctx.eval_with_plaintexts(C, inputs, duration2, EvaluationStrategy::parallel),
				 outputs));
		assert(HAVE_TBB); // shouldn't get here if not build with TBB
		std::cout << "evaluating in parallel took " << duration2[1].count()
			  << " microseconds" << std::endl;
#if ! HAVE_TBB
	} catch (const std::runtime_error&) {
		std::cout << "TBB not defined, caught runtime_error as expected.\n";
	}
#endif
}

int main(void) {
	using namespace Sheep;

	Circuit C;
	Wire a = C.add_input("a");
	Wire b = C.add_input("b");
	Wire c = C.add_input("c");

	Wire a1 = C.add_assignment("a1", Gate::Add, a, b);
	Wire a2 = C.add_assignment("a2", Gate::Add, a, a1);
	Wire a3 = C.add_assignment("a3", Gate::Add, a, a2);

	Wire c1 = C.add_assignment("c1", Gate::Add, b,  c);
	Wire c2 = C.add_assignment("c2", Gate::Add, c1, c);
	Wire c3 = C.add_assignment("c3", Gate::Add, c2, c);
	
	Wire d = C.add_assignment("d", Gate::Add, a3, c3);

	C.set_output(d);

	test<Clear::ContextClear<int8_t> >(C, {1,2,3}, {16});
#ifdef HAVE_HElib
	test<HElib::ContextHElib_F2<int8_t> >(C, {1,2,3}, {16});
#endif

#ifdef HAVE_TFHE
	test<TFHE::ContextTFHE<int8_t> >(C, {1,2,3}, {16});
#endif

}
