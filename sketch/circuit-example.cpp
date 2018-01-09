#include "circuit.hpp"

int main(void)
{
	Circuit c; // construct empty circuit
	const Wire& a = c.add_input("a"); // "a" --> an optional identifier
	const Wire& b = c.add_input("b");
	
	// w2 is the output of th
	const Wire& w2 = c.add_assignment("w2", Gate::And, a, b);

	//c.get_outputs();
}
