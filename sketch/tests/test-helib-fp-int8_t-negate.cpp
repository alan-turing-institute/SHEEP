#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::HElib;
	typedef std::vector<ContextHElib_Fp<int8_t>::Plaintext> PtVec;

	Circuit circ;
	Wire in = circ.add_input("in");
	Wire out = circ.add_assignment("out", Gate::Negate, in);
	circ.set_output(out);

	std::cout<<circ;
	
	ContextHElib_Fp<int8_t> ctx;

	ContextHElib_Fp<int8_t>::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(circ);

	int8_t input = 54;
	std::cout<<"Input "<<std::to_string(input)<<std::endl;
	std::list<ContextHElib_Fp<int8_t>::Plaintext> plaintext_inputs = {input};
	std::list<ContextHElib_Fp<int8_t>::Ciphertext> ciphertext_inputs;
	
	for (ContextHElib_Fp<int8_t>::Plaintext pt: plaintext_inputs)
	  ciphertext_inputs.push_back(ctx.encrypt(pt));
	
	std::list<ContextHElib_Fp<int8_t>::Ciphertext> ciphertext_outputs;
	using microsecond = std::chrono::duration<double, std::micro>;
	microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
	
	std::list<ContextHElib_Fp<int8_t>::Plaintext> plaintext_outputs;
	for (ContextHElib_Fp<int8_t>::Ciphertext ct: ciphertext_outputs) {
	  ContextHElib_Fp<int8_t>::Plaintext pt = ctx.decrypt(ct);
	  plaintext_outputs.push_back(pt);
	  std::cout << std::to_string(pt) << std::endl;
	}	
	assert(plaintext_outputs.front() == -54);

}
