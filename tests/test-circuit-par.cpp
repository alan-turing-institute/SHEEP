#include <cassert>

#include "circuit.hpp"
#include "circuit-util.hpp"
#include "circuit-repo.hpp"

#include "context-clear.hpp"
#include "all_equal.hpp"

int main(void)
{
	CircuitRepo cr;
	NameGenerator names;

	Circuit C = cr.get_circuit_by_name("TestCircuit3");
	std::cout << C << std::endl;

	std::cout << par(C,C) << std::endl;

	assert(false);	
}
