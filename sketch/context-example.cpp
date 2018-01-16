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
	
	std::list<bool> outputs;
	double time = run_circuit({true, false, true, false}, outputs);
	// equivalent to:
	// double time = ctx.eval(c, {true, false}, outputs);
	
	for (auto output : outputs) std::cout << output << std::endl;
}
