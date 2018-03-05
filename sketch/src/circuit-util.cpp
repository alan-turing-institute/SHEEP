#include "circuit-util.hpp"

Circuit seq(const Circuit& first, const Circuit& second)
{
	NameGenerator names;
	Circuit first_copy = copy(first, names);
	Circuit second_copy = copy(second, names);

	// this is a map from output wire names in the first circuit
	// to input wires in the second.
	std::unordered_map<std::string, Wire> splice;
	
	Circuit result;

	// inputs of 'first' are the inputs of the result
	for (auto in : first_copy.get_inputs()) result.add_input(in.get_name());

	// assignments from the first circuit (those from the second
	// follow later)
	for (auto assn : first_copy.get_assignments()) {
		result.add_assignment(assn.get_output().get_name(),
				      assn.get_op(),
				      assn.get_inputs());
	}

	// join the outputs of the first to the inputs of the second via the map 'splice'
	{
		auto first_out_it = first_copy.get_outputs().begin();
		auto second_in_it = second_copy.get_inputs().begin();
		const auto first_out_end = first_copy.get_outputs().end();
		const auto second_in_end = second_copy.get_inputs().end();

		while (first_out_it != first_copy.get_outputs().end()
		       && second_in_it != second_copy.get_inputs().end())
		{
			// The following is a more concise alternative to the
			// 'splicing map', but tends to inflate the generated
			// circuit with aliases.
			//
			// result.add_assignment(second_in_it->get_name(),
			// Gate::Alias, *first_out_it);
			splice.insert({second_in_it->get_name(), *first_out_it});
			++first_out_it;
			++second_in_it;
		}
		if (first_out_it != first_out_end || second_in_it != second_in_end) {
			throw std::runtime_error(
				"seq: when composing circuits, the number of outputs "
				"of the first must equal the number of inputs of the "
				"second");
		}
	}

	// assignments of second
	for (auto assn : second_copy.get_assignments()) {
		Circuit::WireList inputs;
		// translate the inputs of the assignment, using the map splice
		// built above
		for (auto in : assn.get_inputs()) {
			auto new_in = splice.find(in.get_name());
			if (new_in == splice.end()) {
				// not found => the input to the assignment was
				// not an input of the second circuit
				inputs.push_back(in);
			} else {
				inputs.push_back(new_in->second);
			}
		}
		// if the alternative to the splicing map is used, inputs is
		// just assn.get_inputs().
		result.add_assignment(assn.get_output().get_name(),
				      assn.get_op(),
				      inputs);
	}

	// outputs of second are the outputs of the result
	for (auto out : second_copy.get_outputs()) result.set_output(out);
	
	return result;
}

Circuit par(const Circuit& a, const Circuit& b)
{
	NameGenerator names;
	Circuit a1 = copy(a, names);
	Circuit b1 = copy(b, names);

	Circuit result;
	
	for (auto a_in : a1.get_inputs()) result.add_input(a_in.get_name());
	for (auto b_in : b1.get_inputs()) result.add_input(b_in.get_name());

	for (auto a_assn : a1.get_assignments())
		result.add_assignment(a_assn.get_output().get_name(),
				      a_assn.get_op(),
				      a_assn.get_inputs());
	for (auto b_assn : b1.get_assignments())
		result.add_assignment(b_assn.get_output().get_name(),
				      b_assn.get_op(),
				      b_assn.get_inputs());
		
	for (auto a_out : a1.get_outputs()) result.set_output(a_out);
	for (auto b_out : b1.get_outputs()) result.set_output(b_out);

	return result;
}
