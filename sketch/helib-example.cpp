#include <memory>

#include "context-helib.hpp"

int main(void) {
	Circuit C; // construct empty circuit
	const Wire& a = C.add_input("a");
	const Wire& b = C.add_input("b");
	const Wire& c = C.add_input("c");
	const Wire& d = C.add_input("d");
	
	const Wire& w2 = C.add_assignment("w2", Gate::And, a, b);
	const Wire& w3 = C.add_assignment("w3", Gate::Xor, w2, c);

	C.set_output(w3);
	C.set_output(d);

	ContextHElib ctx(20,80);
	
	ContextHElib::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(C);
	
	std::list<ContextHElib::Plaintext> plaintext_inputs = {true, true, true, false};
	std::list<ContextHElib::Ciphertext> ciphertext_inputs;

	for (ContextHElib::Plaintext pt: plaintext_inputs)
	  ciphertext_inputs.push_back(ctx.encrypt(pt));
	
	std::list<ContextHElib::Ciphertext> ciphertext_outputs;
	using microsecond = std::chrono::duration<double, std::micro>;
	microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);

	std::list<ContextHElib::Plaintext> plaintext_outputs;
	for (ContextHElib::Ciphertext ct: ciphertext_outputs) {
	  ContextHElib::Plaintext pt = ctx.decrypt(ct);
	  plaintext_outputs.push_back(pt);
	  std::cout << pt << std::endl;
	}
	std::cout << "time was " << time.count() << " microseconds\n";
}
