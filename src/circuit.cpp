#include "all_equal.hpp"
#include "circuit.hpp"
#include <utility>
#include <map>
#include <sstream>

// Wires are equal iff their names are the same
bool operator==(const Wire& a, const Wire& b)
{
	return a.get_name() == b.get_name();
}

bool operator!=(const Wire& a, const Wire& b)
{
	return !(a == b);
}

template <>
Assignment::Assignment(Wire output_, Gate op_, WireList inputs_)
	: output(output_), op(op_), inputs(inputs_)
{ }

bool operator==(const Assignment& a, const Assignment& b)
{
	// Assignments must perform the same operation:
	if (a.get_op() != b.get_op()) return false;

	// Output wire must match
	if (a.get_output() != b.get_output()) return false;

	// Finally, each input wire must match
	return all_equal(a.get_inputs(), b.get_inputs());
}

bool operator!=(const Assignment& a, const Assignment& b)
{
	return !(a == b);
}

std::ostream& operator<<(std::ostream& stream, const Circuit& c) {

  /// first print out inputs and outputs
  if (c.get_const_inputs().size() > 0) {
    stream<<"CONST_INPUTS";
    for (auto const_input : c.get_const_inputs() ) {
      stream<<" "<<const_input.get_name();
    }
    stream<<std::endl;
  }
  
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
    for (auto const_input : assignment.get_const_inputs()) {
      stream << " " << const_input.get_name();
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

std::istream& operator >>(std::istream& stream, Circuit& c)
{
  /// loop over all lines in the input file 
  std::string line;

  // pairs of the circuit outputs and the line number where they were
  // found in the input
  std::vector<std::pair<int, std::string> > circuit_outputs;

  int lineno = 1; 
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
      if (*token_iter == "CONST_INPUTS") {
	token_iter++;
	for (; token_iter != tokens.end(); ++token_iter) {
	  c.add_const_input(*token_iter);
	}
      } else if (*token_iter == "INPUTS") {
	token_iter++;
	for (; token_iter != tokens.end(); ++token_iter) {
	  try {
	    c.add_input(*token_iter);
	  }
	  catch (const MultipleAssignmentError& e) {
	    std::cerr << "Error reading circuit: "
	              << "line " << lineno << ": in INPUTS: a wire named '" << e.name
		      << "' has already been used to store a value (it must be unique).\n";
	    throw e;
	  };
	}
      } else if (*token_iter == "OUTPUTS") {
	token_iter++;
	for (; token_iter != tokens.end(); ++token_iter) {
	  // store outputs, to be set at the end, or an UndefinedVariableError will be thrown.
	  circuit_outputs.emplace_back(lineno, *token_iter);
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
	  auto it = gate_name_map.find(gate_name);
	  if (it != gate_name_map.end()) {
	    Gate gate = it->second;
	    try {
	      const Wire& gateout = c.add_assignment(gate_output_names.front(),
						     gate,
						     gate_inputs);
	    }
	    catch (const MultipleAssignmentError& e) {
	      std::cerr << "Error reading circuit: "
		        << "line " << lineno << ": the wire named '" << e.name
			<< "' has already been used to store a value (it must be unique).\n";
	      throw e;
	    }
	    catch (const UndefinedVariableError& e) {
	      std::cerr << "Error reading circuit: "
			<< "line " << lineno << ": a wire named '" << e.name
			<< "' has not yet been defined\n";
	      throw e;
	    };
	  }
	}	
      }
    }
    ++lineno;
  } /// end of loop over input lines

  // finally add the outputs to the circuit
  for (std::pair<int, std::string> line_wire_pair: circuit_outputs) {
    try {
      c.set_output(line_wire_pair.second);
    }
    catch (const UndefinedVariableError& e) {
      std::cerr << "Error reading circuit: "
		<< "line " << line_wire_pair.first << ": in OUTPUTS: cannot set a wire named '" << e.name
		<< "' to be an output of the circuit as one with this name was not defined.\n";
      throw e;
    }
  }
  return stream;
}

bool operator==(const Circuit& c1, const Circuit& c2)
{
	/// check individual inputs, outputs and assignments
	bool inputs_match = all_equal(c1.get_inputs(), c2.get_inputs());
	bool outputs_match = all_equal(c1.get_outputs(), c2.get_outputs());
	bool assignments_match = all_equal(c1.get_assignments(), c2.get_assignments());
	return inputs_match && outputs_match && assignments_match;
}

bool operator!=(const Circuit& c1, const Circuit& c2)
{
	return !(c1 == c2);
}
