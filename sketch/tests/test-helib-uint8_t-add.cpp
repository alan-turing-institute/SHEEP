#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"

int main(void) {
	using namespace Sheep::HElib;

    //// instantiate the Circuit Repository
	CircuitRepo cr;

	Circuit circ = cr.create_circuit(Gate::Add, 1);
	std::cout << circ;

	ContextHElib<uint8_t> ctx(20,80);

	ContextHElib<uint8_t>::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(circ);
	
	std::list<ContextHElib<uint8_t>::Plaintext> plaintext_inputs = {10,12};
	std::list<ContextHElib<uint8_t>::Ciphertext> ciphertext_inputs;

	for (ContextHElib<uint8_t>::Plaintext pt: plaintext_inputs)
	  ciphertext_inputs.push_back(ctx.encrypt(pt));

	std::list<ContextHElib<uint8_t>::Ciphertext> ciphertext_outputs;	

	using microsecond = std::chrono::duration<double, std::micro>;
	microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);

	std::list<ContextHElib<uint8_t>::Plaintext> plaintext_outputs;
	for (ContextHElib<uint8_t>::Ciphertext ct: ciphertext_outputs) {
	  ContextHElib<uint8_t>::Plaintext pt = ctx.decrypt(ct);
	  plaintext_outputs.push_back(pt);
	}

	assert(plaintext_outputs.front() == 22);

}
