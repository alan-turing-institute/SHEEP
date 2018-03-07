#include "circuit-repo.hpp"
#include <fstream>



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
  
  Circuit c = cr.create_circuit(gate_name_map[gate_name],depth);

  std::ofstream outfile(output_filename);
  outfile << c;
  
}
