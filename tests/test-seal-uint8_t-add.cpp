#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-seal.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
	using namespace Sheep::Seal;

    //// instantiate the Circuit Repository
	CircuitRepo cr;

	Circuit circ = cr.create_circuit(Gate::Add, 1);
	std::cout << circ;
	std::vector<DurationT> durations;
	ContextSeal<uint8_t> ctx;

	ContextSeal<uint8_t>::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(circ);

/// test small postitive numbers
        std::vector<uint8_t> inputs = {5, 22};
        std::vector<uint8_t> result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" 5+22 = "<<std::to_string(result.front())<<std::endl;      
        assert(result.front() == 27);

        /// test result going out of range positive
        inputs = {200, 127};
        result = ctx.eval_with_plaintexts(circ, inputs, durations);
        std::cout<<" 200 + 127 = "<<std::to_string(result.front())<<std::endl;
        assert(result.front() == 71);

}
