#include "circuit-repo.hpp"
#include <fstream>
#include <iostream>


int main(int argc, const char** argv)
{
  if (argc < 4) {
    std::cout<<"Usage: simple-circuit-maker <gate> <depth> <output_filename>"<<std::endl;
    return 0;

  }
  std::string gate_name = argv[1];

  int depth = std::stoi(argv[2]);

  std::string output_filename = argv[3];

  
  CircuitRepo cr;

  auto it = gate_name_map.find(gate_name);
  if (it != gate_name_map.end()) {
    Gate gate = it->second;
    Circuit c = cr.create_circuit(gate,depth);

    std::ofstream outfile(output_filename);
    outfile << c;
  } else {
    throw std::runtime_error("gate name not found");
  }
}
