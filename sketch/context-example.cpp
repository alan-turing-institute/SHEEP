#include <memory>

#include "context-clear.hpp"

int main(void)
{
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
	
	CircuitEvaluator run_circuit;
	run_circuit = ctx.compile(C);
	
	std::list<ContextClear::Ciphertext> secret_outputs;
	std::list<ContextClear::Ciphertext> ciphertexts = ctx.encrypt({true, false, true, false});

	double time = run_circuit(ciphertexts, secret_outputs); // outputs :: list<Ciphertext>

	std::list<ContextClear::Plaintext> public_outputs = ctx.decrypt(secret_outputs);


// equivalent to:
	//double time = ctx.eval(C, {true, false}, outputs);
	//C.assert({}, 0)
	//for (auto output : outputs) std::cout << output << std::endl;

	//TestClear.test(C, {a: 1. b: 1}, {0,1})
	//	assert(output[0] == true);
	
}

// Slots per ciphertext
