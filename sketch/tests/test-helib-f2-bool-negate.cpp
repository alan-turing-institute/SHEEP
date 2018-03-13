#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::HElib;
	typedef std::vector<ContextHElib_F2<bool>::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	std::cout<<circ;
	
	ContextHElib_F2<bool> ctx;

	ContextHElib_F2<bool>::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(circ);

	bool input = true;
	std::cout<<"Input "<<std::to_string(input)<<std::endl;
	std::list<ContextHElib_F2<bool>::Plaintext> plaintext_inputs = {input};
	std::list<ContextHElib_F2<bool>::Ciphertext> ciphertext_inputs;
	
	for (ContextHElib_F2<bool>::Plaintext pt: plaintext_inputs)
	  ciphertext_inputs.push_back(ctx.encrypt(pt));
	
	std::list<ContextHElib_F2<bool>::Ciphertext> ciphertext_outputs;
	using microsecond = std::chrono::duration<double, std::micro>;
	microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
	
	std::list<ContextHElib_F2<bool>::Plaintext> plaintext_outputs;
	for (ContextHElib_F2<bool>::Ciphertext ct: ciphertext_outputs) {
	  ContextHElib_F2<bool>::Plaintext pt = ctx.decrypt(ct);
	  plaintext_outputs.push_back(pt);
	  std::cout << std::to_string(pt) << std::endl;
	}	
	assert(plaintext_outputs.front() == false);

}
