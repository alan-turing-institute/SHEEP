#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <string>
#include <list>
#include <iostream>


class Wire {
	std::string name;
	// this class represents 'wires' (1-bit) or 'buses'
public:
	Wire(std::string name_) : name(name_) { }
	const std::string get_name() const { return name; }
};

enum class Gate {And, Or, Xor};

// class ConstWire : public Wire {
// };

class Assignment {
	Wire output;
	Gate op;
	// std::list<const Wire&> inputs;
	Wire input1, input2;
public:
	Assignment(Wire output_, Gate op_, Wire input1_, Wire input2_)
		: output(output_), op(op_), input1(input1_), input2(input2_)
	{ }
	Wire get_input1() const { return input1; }
	Wire get_input2() const { return input2; }
	Wire get_output() const { return output; }
	Gate get_op() const { return op; }
};

struct Output {
	Wire wire;
	Output(Wire w) : wire(w) {}
};

class Circuit {
	// probably don't need to write an eval method here
	std::list<Wire> inputs;
	std::list<Wire> wires;
	std::list<Output> outputs;
	std::list<Assignment> assignments;
public:

	Wire add_input(std::string name) {
		inputs.emplace_back(name);
		return inputs.back();
	}

	Wire add_assignment(std::string name, Gate op, Wire a, Wire b) {
		wires.emplace_back(name);
		Wire output = wires.back();
		assignments.emplace_back(output, op, a, b);
	        return output;
	}

	void set_output(Wire w) {
		outputs.emplace_back(w);
	}

	const std::list<Assignment>& get_assignments() const {
		return assignments;
	}

	const std::list<Wire>& get_inputs() const {
		return inputs;
	}

  	const std::list<Wire>& get_wires() const {
		return wires;
	}

	const std::list<Output>& get_outputs() const {
		return outputs;
	}

	void print() const {
		for (auto assignment : assignments) {
			std::cout << "Assignment:  " << std::endl
				  << "   " << assignment.get_input1().get_name()
				  << " " << assignment.get_input2().get_name()
				  << "  --> " << assignment.get_output().get_name()
				  << std::endl;
		}
	}
};

#endif // define CIRCUIT_HPP
