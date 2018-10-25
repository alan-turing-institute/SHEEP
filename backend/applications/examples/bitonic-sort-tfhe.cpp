#include <cstdint>
#include <iostream>
#include "simple-circuits.hpp"
//#include "context-clear.hpp"
#include "context-tfhe.hpp"
#include "all_equal.hpp"

int main(void)
{
	using namespace SHEEP;
	typedef std::vector<std::vector<ContextTFHE<int8_t>::Plaintext>> PtVec;

	std::cout << "Constructing context...\n";
	ContextTFHE<int8_t> ctx;


	std::cout << "The bitonic sorting network is:\n";
	Circuit bitonic = bitonic_sort(4, false);
	std::cout << bitonic << std::endl;

	PtVec inputs {{7}, {3}, {10}, {0}};

	std::cout << "Inputs are: ";
	for (auto x : inputs) std::cout << std::to_string(x[0]) << " ";
	std::cout << std::endl;

	PtVec sorted = ctx.eval_with_plaintexts(bitonic, inputs);

	std::cout << "Sorted result is: ";
	for (auto x : sorted) std::cout << std::to_string(x[0]) << " ";
	std::cout << std::endl;
}
