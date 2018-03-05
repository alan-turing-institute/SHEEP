#include <cstdint>
#include <iostream>
#include "simple-circuits.hpp"
//#include "context-clear.hpp"
#include "context-tfhe.hpp"
#include "all_equal.hpp"

int main(void)
{
	using namespace Sheep::TFHE;
	typedef std::vector<ContextTFHE<int8_t>::Plaintext> PtVec;

	std::cout << "Constructing context...\n";
	ContextTFHE<int8_t> ctx;

	std::cout << "The bitonic sorting network is:\n";
	Circuit bitonic = bitonic_sort(4, false);
	std::cout << bitonic << std::endl;

	PtVec inputs {7, 3, 10, 0};

	std::cout << "Inputs are: ";
	for (auto x : inputs) std::cout << std::to_string(x) << " ";
	std::cout << std::endl;

	PtVec sorted = eval_with_plaintexts(ctx, bitonic, inputs);

	std::cout << "Sorted result is: ";
	for (auto x : sorted) std::cout << std::to_string(x) << " ";
	std::cout << std::endl;
}
