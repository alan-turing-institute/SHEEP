#include <cassert>
#include <fstream>
#include "circuit.hpp"

int main(void)
{
	std::ifstream good_circuit_file("good.sheep");
	assert(good_circuit_file.is_open());
	Circuit C_good;
	good_circuit_file >> C_good;
	
	return 0;
}
