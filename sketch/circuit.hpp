#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <string>
#include <list>


class Wire {
	std::string name;
	// this class represents 'wires' (1-bit) or 'buses'
public:
	Wire(std::string name_) : name(name_) { }	       
};


enum class Gate {And};


// class ConstWire : public Wire {
// };

class Assignment {
	const Wire& output;
	Gate op;
	// std::list<const Wire&> inputs;
	const Wire& input1, input2;
public:
	Assignment(const Wire& output_, Gate op_, const Wire& input1_, const Wire& input2_)
		: output(output_), op(op_), input1(input1_), input2(input2_)
	{ }
};

class Circuit {
	// probably don't need to write an eval method here
	std::list<Wire> inputs;
	std::list<Wire> wires;
	std::list<Wire> outputs;
	std::list<Assignment> assignments;
public:
	const Wire& add_input(std::string name) {
		inputs.emplace_back(name);
		return inputs.back();
	}

	// const Wire& add_and_gate(const Wire& a, const Wire& b) {
	// 	std::vector<const Wire&> add_assignment(Gate::And, std::vector<const Wire&>(a, b));
	// }
	const Wire& add_assignment(std::string name, Gate op, const Wire& a, const Wire& b) {
		wires.emplace_back(name);
		const Wire& output = wires.back();
		assignments.emplace_back(output, op, a, b);
		
		return output;
	}
};

#endif // define CIRCUIT_HPP
