#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <map>

enum class Gate {Multiply, Maximum, Add, Subtract, Negate, Compare};

static std::map<std::string, Gate> gate_name_map = {
  {"ADD", Gate::Add },
  {"MULTIPLY", Gate::Multiply },
  {"SUBTRACT", Gate::Subtract },
  {"MAXIMUM", Gate::Maximum },
  {"NEGATE", Gate::Negate },
  {"COMPARE", Gate::Compare }
};


class Wire {
	std::string name;
public:
	Wire(std::string name_) : name(name_) { }
	const std::string get_name() const { return name; }
};



class Assignment {
public:
	typedef std::vector<Wire> WireList;
private:
	Wire output;
	Gate op;
	WireList inputs;
public:
	template <typename... Ts>
	Assignment(Wire output_, Gate op_, Ts... inputs_)
		: output(output_), op(op_), inputs{inputs_...}
	{ }
	size_t input_count() const { return inputs.size();  }
	const WireList& get_inputs() const { return inputs; }
	Wire get_output() const { return output; }
	Gate get_op() const { return op; }
};

class Circuit {
public:
	typedef std::list<Wire> WireList;
	typedef std::list<Assignment> AssignmentList;
private:
	WireList inputs;
	WireList wires;
	WireList outputs;
	AssignmentList assignments;
public:

	Wire add_input(std::string name) {
		inputs.emplace_back(name);
		return inputs.back();
	}

	template <typename... Wires>
	Wire add_assignment(std::string name, Gate op, Wires... ws) {
		wires.emplace_back(name);
		Wire output = wires.back();
		assignments.emplace_back(output, op, ws...);
	        return output;
	}

	void set_output(Wire w) {
		outputs.emplace_back(w);
	}

	const AssignmentList& get_assignments() const {
		return assignments;
	}

	const WireList& get_inputs() const {
		return inputs;
	}

	const WireList& get_wires() const {
		return wires;
	}

	const WireList& get_outputs() const {
		return outputs;
	}
};

std::ostream& operator<<(std::ostream& ostream, const Circuit& c);

std::istream& operator >>(std::istream& istream, Circuit& c);

#endif // define CIRCUIT_HPP
