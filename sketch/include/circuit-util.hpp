#ifndef CIRCUIT_UTIL_HPP
#define CIRCUIT_UTIL_HPP

#include <string>
#include <sstream>

#include "circuit.hpp"

class NameGenerator {
	size_t counter;
public:
	NameGenerator() : counter(0) { }
	std::string operator()(std::string prefix) {
		std::stringstream ss;
		ss << prefix << counter++;
		return ss.str();
	}
};

// Given a Circuit C, produce a new circuit equivalent to C, but whose
// inputs, outputs and assigments are named with unique strings
// produced by repeated calls to name(prefix), where prefix is a
// std::string.
template <typename NameGenT>
Circuit copy(const Circuit& C, NameGenT& name)
{
	Circuit C_copy;
	
	// 'translate' serves as the table mapping Wire names in the
	// original Circuit to Wires in the copy
	std::unordered_map<std::string, Wire> translate;

	// new inputs
	for (const Wire wire : C.get_inputs()) {
		const Wire wire_copy = C_copy.add_input(name("i"));
		translate.insert({wire.get_name(), wire_copy});
	}

	// new assignments
	for (const Assignment assn : C.get_assignments()) {
		Assignment::WireList new_inputs;
		for (const Wire orig_input_wire : assn.get_inputs()) {
			new_inputs.emplace_back(translate.at(orig_input_wire.get_name()));
		}
		// the result wire of this assignment
		Wire assn_result = C_copy.add_assignment(name("w"),
							 assn.get_op(),
							 new_inputs);
		translate.insert({assn.get_output().get_name(), assn_result});
	}

	// new circuit outputs
	for (const Wire output : C.get_outputs()) {
		C_copy.set_output(translate.at(output.get_name()));
	}

	return C_copy;
}

Circuit seq(const Circuit&, const Circuit&);
Circuit par(const Circuit&, const Circuit&);

#endif //CIRCUIT_UTIL_HPP
