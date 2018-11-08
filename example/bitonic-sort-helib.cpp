#include <cstdint>
#include <fstream>
#include <iostream>
#include "all_equal.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

int main(void) {
  using namespace SHEEP;
  typedef std::vector<ContextHElib_F2<int8_t>::Plaintext> PtVec;

  std::cout << "Constructing context...\n";
  ContextHElib_F2<int8_t> ctx;  // paramset, bootstrappable

  std::cout << "The bitonic sorting network is:\n";
  Circuit bitonic = bitonic_sort(4, false);
  std::cout << bitonic << std::endl;
  std::ofstream outputfile("bitonic_sort.sheep");
  outputfile << bitonic;
  outputfile.close();

  PtVec inputs{7, 3, 22, 2};

  std::cout << "Inputs are: ";
  for (auto x : inputs) std::cout << std::to_string(x) << " ";
  std::cout << std::endl;

  PtVec sorted = ctx.eval_with_plaintexts(bitonic, inputs);

  std::cout << "Sorted result is: ";
  for (auto x : sorted) std::cout << std::to_string(x) << " ";
  std::cout << std::endl;
}
