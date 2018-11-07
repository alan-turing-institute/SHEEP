#include <fstream>
#include <map>
#include <sstream>
#include "circuit.hpp"

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cout << "Please specify a filename" << std::endl;
    return -1;
  }
  std::string filename = argv[1];

  std::ifstream circuit_file(filename);
  Circuit c;  // construct empty circuit
  circuit_file >> c;

  std::cout << c;
  std::cout << " Number of inputs " << c.get_inputs().size()
            << "  Number of outputs " << c.get_outputs().size() << std::endl;

  circuit_file.close();
  return 0;
}
