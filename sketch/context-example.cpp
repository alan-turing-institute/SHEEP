#include <memory>

#include "context-clear.hpp"

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

	ContextClear ctx;
	
	ContextClear::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(C);
	
	std::list<ContextClear::Plaintext> plaintext_inputs = {true, false, true, false};
	std::list<ContextClear::Ciphertext> ciphertext_inputs;

	for (ContextClear::Plaintext pt: plaintext_inputs)
		ciphertext_inputs.push_back(ctx.encrypt(pt));
	
	std::list<ContextClear::Ciphertext> ciphertext_outputs;
	using microsecond = std::chrono::duration<double, std::micro>;
	microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);

	std::list<ContextClear::Plaintext> plaintext_outputs;
	for (ContextClear::Ciphertext ct: ciphertext_outputs) {
		ContextClear::Plaintext pt = ctx.decrypt(ct);
		plaintext_outputs.push_back(pt);
		std::cout << pt << std::endl;
	}
	std::cout << "time was " << time.count() << " microseconds\n";
}
