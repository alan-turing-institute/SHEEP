#include "circuit.hpp"
#include <map>
#include <sstream>  




std::ostream& operator<<(std::ostream& stream, const Circuit& c) {

  /// first print out inputs and outputs
  
  stream<<"INPUTS";
  for (auto input : c.get_inputs() ) {
    stream<<" "<<input.get_name();
  }
  stream<<std::endl;
  stream<<"OUTPUTS";
  
  for (auto output : c.get_outputs() ) {
    stream<<" "<<output.get_name();
  }
  stream<<std::endl;

  /// now loop through assignments
  
  for (auto assignment : c.get_assignments()) {

    for (auto input : assignment.get_inputs()) {
      stream << " " << input.get_name();
    }
    std::string gate_name = "";
    for (auto map_it = gate_name_map.begin(); map_it != gate_name_map.end(); ++map_it ) {
      if (map_it->second == assignment.get_op()) {
	gate_name = map_it->first;
	break;
      }
    }

    stream << " "<<gate_name;
    stream << " " << assignment.get_output().get_name();
    stream << "\n";
  }
  return stream;
}

std::istream& operator >>(std::istream& stream, Circuit& c) {


  /// loop over all lines in the input file 
  std::string line;
  while (std::getline(stream, line) ) {
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
	  c.add_input(*token_iter);
	}
      } else if (*token_iter == "OUTPUTS") {
	token_iter++;
	for (; token_iter != tokens.end(); ++token_iter) {
	  c.set_output(*token_iter);
	}
      } else {   /// we are in the assignments block - format is:
	/// input1 input2 ... inputN GATE output1 [... outputN]
	
	std::vector<Wire> gate_inputs;
	std::vector<std::string> gate_output_names;            
	std::string gate_name = "";

	/// iterate over all tokens - the tokens before the gate name are inputs, those after are outputs.
	bool found_gate = false;
	
	for ( const std::string token : tokens) {
	  if (gate_name_map.find(token) != gate_name_map.end()) {
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
	  const Wire& gateout = c.add_assignment(gate_output_names.front(),
						 gate_name_map[gate_name],
						 gate_inputs);
	}	
      }
    }
    
  } /// end of loop over input lines
  
  return stream;
}
