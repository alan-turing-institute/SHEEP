#include <memory>
#include <cassert>
#include <list>

#include <fstream>

#include "circuit-repo.hpp"

bool check_circuits_equal(const Circuit& c1, const Circuit& c2) {

  bool inputs_same_length= (c1.get_inputs().size() == c2.get_inputs().size());
  if (! inputs_same_length) return false;

  bool outputs_same_length= (c1.get_outputs().size() == c2.get_outputs().size());
  if (! outputs_same_length) return false;

  bool assignments_same_length= (c1.get_assignments().size() == c2.get_assignments().size());
  if (! assignments_same_length) return false;  

  /// check individual inputs and outputs
  
  Circuit::WireList c1_inputs = c1.get_inputs();
  Circuit::WireList c2_inputs = c2.get_inputs();
  auto c1_it = c1_inputs.begin();
  auto c2_it = c2_inputs.begin();  
  while( c1_it != c1_inputs.end() && c2_it != c2_inputs.end() ) {
    if (c1_it->get_name() != c2_it->get_name()) return false;
    c1_it++;
    c2_it++;
  }

  Circuit::WireList c1_outputs = c1.get_outputs();
  Circuit::WireList c2_outputs = c2.get_outputs();
  auto c1out_it = c1_outputs.begin();
  auto c2out_it = c2_outputs.begin();
  while( c1out_it != c1_outputs.end() && c2out_it != c2_outputs.end() ) {
    if (c1out_it->get_name() != c2out_it->get_name()) return false;
    c1out_it++;
    c2out_it++;
  }
  
  return true;
}


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
  
  assert(check_circuits_equal(C_orig,C_new));

 
}
