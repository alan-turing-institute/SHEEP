#include "context-clear.hpp"

int main(void)
{
	Circuit c; // construct empty circuit
	const Wire& a = c.add_input("a");
	const Wire& b = c.add_input("b");
	
	// w2 is the output of th
	const Wire& w2 = c.add_assignment("w2", Gate::And, a, b);

	c.set_output(w2);
	
	ContextClear ctx;
	
	std::list<bool> outputs;

	double time = ctx.eval(c, {true, false}, outputs);

	for (auto output : outputs) std::cout << output << std::endl;
	
}
