#include <cstdint>
#include <iostream>
#include <fstream>
#include "simple-circuits.hpp"
//#include "context-clear.hpp"
#include "context-helib.hpp"
#include "all_equal.hpp"

int main(void)
{
	using namespace Sheep::HElib;
	typedef std::vector<ContextHElib<int8_t>::Plaintext> PtVec;

	std::cout << "Constructing context...\n";
	ContextHElib<int8_t> ctx(2,true);   // paramset, bootstrappable

	std::cout << "The bitonic sorting network is:\n";
	Circuit bitonic = bitonic_sort(4, false);
	std::cout << bitonic << std::endl;
	std::ofstream outputfile("bitonic_sort.sheep");
	outputfile << bitonic;
	outputfile.close();
	
	PtVec inputs {7, 3, 22, 2 };

	std::cout << "Inputs are: ";
	for (auto x : inputs) std::cout << std::to_string(x) << " ";
	std::cout << std::endl;

	PtVec sorted = eval_with_plaintexts(ctx, bitonic, inputs);

	std::cout << "Sorted result is: ";
	for (auto x : sorted) std::cout << std::to_string(x) << " ";
	std::cout << std::endl;
}
