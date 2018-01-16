#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include <unordered_map>

#include "circuit.hpp"
#include "context.hpp"

class ContextClear : public Context {
public:
	
	bool And(bool a, bool b) {
		return a & b;
	}

	std::function<bool(bool,bool)> get_op(Gate g) {
		switch(g) {
		case(Gate::And):
			std::function<bool(bool,bool)> op;
			op = [this](bool a, bool b) mutable { return And(a,b); };
			return op;
			break;
		}
		throw std::runtime_error("Unknown op");
	}

	// each Context concrete class provides its own compile
	// method, which can perform any library-specific
	// optimization.
	bool eval(const Circuit& circ, const std::list<bool>& input_vals, double& t) {

		// build map of Wire& -> bool
		std::unordered_map<std::string, bool> eval_map;

		// add Circuit::inputs and inputs into the map
		auto input_vals_it = input_vals.begin();
		auto input_wires_it = circ.get_inputs().begin();
		for (; input_vals_it != input_vals.end() || input_wires_it != circ.get_inputs().end();
		     ++input_vals_it, ++input_wires_it)
		{
			eval_map.insert({input_wires_it->get_name(), *input_vals_it});
		}

		// error check: both iterators should be at the end
		// ...
		
		// for each assignment, look up the Wire in the map
		for (const Assignment assn : circ.get_assignments()) {

			// throws out_of_range if not present in the map
			bool input1 = eval_map.at(assn.get_input1().get_name());
			bool input2 = eval_map.at(assn.get_input2().get_name());
			auto op = get_op(assn.get_op());
			bool output = op(input1, input2);
		}
		t = 0.0;
		return true;
	}
};



// context.eval(circ, inputs, time);

// evaluator = context.compile(circ);
// evaluator(inputs, time);

#endif // CONTEXT_CLEAR_HPP
