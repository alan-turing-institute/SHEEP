#include "context-clear.hpp"

int main(void)
{
	Circuit c; // construct empty circuit
	const Wire& a = c.add_input("a");
	const Wire& b = c.add_input("b");
	
	// w2 is the output of th
	const Wire& w2 = c.add_assignment("w2", Gate::And, a, b);

	ContextClear ctx;

	double time;
	bool output = ctx.eval(c, {true, true}, time);

	std::cout << output << std::endl;
	
}
