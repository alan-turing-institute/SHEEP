#include "circuit.hpp"
#include <fstream>
#include <map>
#include <sstream>


int main(int argc, char** argv)
{
  if (argc <= 1) {
    std::cout<<"Please specify a filename" <<std::endl;
    return -1;
  }
  std::string filename = argv[1];
  
  std::map<std::string, Gate> known_gates;
  known_gates.insert({"ADD",Gate::Add});
  known_gates.insert({"MULTIPLY",Gate::Multiply});
  known_gates.insert({"SUBTRACT",Gate::Subtract});    
  
  
  std::ifstream circuit_file(filename);
  Circuit c; // construct empty circuit

  /// loop over all lines in the input file 
  std::string line;
  while (std::getline(circuit_file, line) ) {
    /// remove comments (lines starting with #) and empty lines
    int found= line.find_first_not_of(" \t");
    if( found != std::string::npos) {   
      if ( line[found] == '#') 
	continue;
      
      /// split up by whitespace
      std::string buffer;
      std::vector<std::string> tokens;
      std::stringstream ss(line);
      while (ss >> buffer) tokens.push_back(buffer);
      
      ////  see if the first word in the line is INPUTS or OUTPUTS
      auto token_iter = tokens.begin();
      if (*token_iter == "INPUTS") {
	token_iter++;
	for (; token_iter != tokens.end(); ++token_iter) {
	  std::cout<<"Adding "<<*token_iter<<" to inputs"<<std::endl;
	  c.add_input(*token_iter);
	}
      } else if (*token_iter == "OUTPUTS") {
	token_iter++;
	for (; token_iter != tokens.end(); ++token_iter) {
	  std::cout<<"Adding "<<*token_iter<<" to outputs"<<std::endl;
	  c.set_output(*token_iter);
	}
      } else {   /// we are in the assignments block - format
	/// input1 input2 ... inputN GATE output1 [... outputN]
	
	std::vector<Wire> gate_inputs;
	std::vector<std::string> gate_output_names;            
	std::string gate_name = "";

	/// iterate over all tokens - the tokens before the gate name are inputs, those after are outputs.
	bool found_gate = false;
	
	for ( const std::string token : tokens) {
	  if (known_gates.find(token) != known_gates.end()) {
	    gate_name = token;
	    found_gate = true;
	  } else if (! found_gate) {
	    gate_inputs.emplace_back(token);
	  } else {
	    gate_output_names.push_back(token);
	  }
	}
	if (found_gate && gate_inputs.size() > 0 && gate_output_names.size() > 0) {
	  /// add this assignment
	  const Wire& gateout = c.add_assignment(gate_output_names.front(), known_gates[gate_name], gate_inputs);
	}	
      }
    }
    
  } /// end of loop over input lines
  
  std::cout << c;
  std::cout<<" Number of inputs "<<c.get_inputs().size()<<"  Number of outputs "<<c.get_outputs().size()<<std::endl;
  
  circuit_file.close();
  return 0;
}

