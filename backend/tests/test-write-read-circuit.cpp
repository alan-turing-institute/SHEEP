#include <cassert>
#include <list>
#include <memory>

#include <fstream>

#include "circuit-repo.hpp"

int main(void) {
  std::string filename = "test_circuit.circ";

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  //// build a circuit with a specified depth of a specified gate
  Circuit C_orig = cr.get_circuit_by_name("TestCircuit3");

  std::ofstream outfile(filename);

  outfile << C_orig;

  outfile.close();

  /// now read it back in

  Circuit C_new;
  std::ifstream infile(filename);
  infile >> C_new;

  assert(C_orig == C_new);
}
