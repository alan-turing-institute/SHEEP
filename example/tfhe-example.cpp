#include <memory>

#include "context-tfhe.hpp"

int main(void) {
	Circuit C; // construct empty circuit
	const Wire& a = C.add_input("a");
	const Wire& b = C.add_input("b");
	const Wire& c = C.add_input("c");
	const Wire& d = C.add_input("d");
	
	const Wire& w2 = C.add_assignment("w2", Gate::Multiply, a, b);
	const Wire& w3 = C.add_assignment("w3", Gate::Add, w2, c);

	C.set_output(w3);
	C.set_output(d);

	using namespace SHEEP;
	
	ContextTFHE<bool> ctx;
	
	ContextTFHE<bool>::CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(C);
	
	std::list<ContextTFHE<bool>::Plaintext> plaintext_inputs = {false, true, true, true};
	std::list<ContextTFHE<bool>::Ciphertext> ciphertext_inputs;

	for (ContextTFHE<bool>::Plaintext pt: plaintext_inputs)
	  ciphertext_inputs.push_back(ctx.encrypt(pt));
	
	std::list<ContextTFHE<bool>::Ciphertext> ciphertext_outputs;
	using microsecond = std::chrono::duration<double, std::micro>;
	microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);

	std::list<ContextTFHE<bool>::Plaintext> plaintext_outputs;
	for (ContextTFHE<bool>::Ciphertext ct: ciphertext_outputs) {
	  ContextTFHE<bool>::Plaintext pt = ctx.decrypt(ct);
	  plaintext_outputs.push_back(pt);
	  std::cout << pt << std::endl;
	}
	std::cout << "time was " << time.count() << " microseconds\n";
}
