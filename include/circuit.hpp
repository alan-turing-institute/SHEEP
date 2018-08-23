#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <map>
#include <unordered_set>

enum class Gate {Alias, Identity, Multiply, Maximum, Add, Subtract, Negate, Compare, Select, MultByConstant, AddConstant};

static const std::map<std::string, Gate> gate_name_map = {
  {"ALIAS", Gate::Alias },
  {"ID", Gate::Identity },
  {"ADD", Gate::Add },
  {"MULTIPLY", Gate::Multiply },
  {"SUBTRACT", Gate::Subtract },
  {"MAXIMUM", Gate::Maximum },
  {"NEGATE", Gate::Negate },
  {"COMPARE", Gate::Compare },
  {"SELECT", Gate::Select },
  {"MULTBYCONST", Gate::MultByConstant },
  {"ADDCONST", Gate::AddConstant }  
};


class Wire {
	std::string name;
public:
	Wire(std::string name_) : name(name_) { }
	const std::string get_name() const { return name; }
};

bool operator==(const Wire&, const Wire&);
bool operator!=(const Wire&, const Wire&);


class Assignment {
public:
	typedef std::vector<Wire> WireList;
private:
	Wire output;
	Gate op;
	WireList inputs;
        WireList const_inputs;
public:
	template <typename... Ts>
	Assignment(Wire output_, Gate op_, Ts... inputs_);

	size_t input_count() const { return inputs.size();  }
	const WireList& get_inputs() const { return inputs; }
        const WireList& get_const_inputs() const { return const_inputs; }
	Wire get_output() const { return output; }
	Gate get_op() const { return op; }
};

template <typename... Ts>
Assignment::Assignment(Wire output_, Gate op_, Ts... inputs_)
	: output(output_), op(op_), inputs{inputs_...}
{ }

template <>
Assignment::Assignment(Wire, Gate, Assignment::WireList);

bool operator==(const Assignment&, const Assignment&);
bool operator!=(const Assignment&, const Assignment&);


struct MultipleAssignmentError : public std::runtime_error {
	const std::string name;
	MultipleAssignmentError(std::string name_)
		: name(name_),
		  std::runtime_error("Multiple assignment to wire named '" + name_ + "'")
	{ };
};

struct UndefinedVariableError : public std::runtime_error {
	const std::string name;
	UndefinedVariableError(std::string name_)
		: name(name_),
		  std::runtime_error("A wire with name '" + name_ + "' has not yet been defined.")
	{ };
};

class Circuit {
public:
	typedef Assignment::WireList WireList;
	typedef std::vector<Assignment> AssignmentList;
private:
	// The unordered_sets of their names allow efficient lookup
	// for validation of the circuit.  Contains names found in
	// 'inputs' and 'wires'.
	std::unordered_set<std::string> wire_names;

	WireList inputs;
	WireList wires;

	WireList outputs;

     WireList const_inputs;
	AssignmentList assignments;
public:

	Wire add_input(std::string name) {
		std::unordered_set<std::string>::iterator it_ignored;
		bool inserted;
		std::tie(it_ignored, inserted) = wire_names.insert(name);
		if (!inserted) throw MultipleAssignmentError(name);
		
		inputs.emplace_back(name);
		return inputs.back();
	}

  	Wire add_const_input(std::string name) {
  		std::unordered_set<std::string>::iterator it_ignored;
		bool inserted;
		std::tie(it_ignored, inserted) = wire_names.insert(name);
		if (!inserted) throw MultipleAssignmentError(name);
		const_inputs.emplace_back(name);
		return const_inputs.back();
	}

	template <typename... Wires>
	Wire add_assignment(std::string name, Gate op, Wires... ws) {
		std::unordered_set<std::string>::iterator it_ignored;
		bool inserted;
		std::tie(it_ignored, inserted) = wire_names.insert(name);
		if (!inserted) throw MultipleAssignmentError(name);
		
		wires.emplace_back(name);
		Wire output = wires.back();

		Assignment assgn(output, op, ws...);
		for (auto assgn_in : assgn.get_inputs()) {
			if (wire_names.count(assgn_in.get_name()) == 0) {
				throw UndefinedVariableError(assgn_in.get_name());
			}
		}
		assignments.push_back(std::move(assgn));
		
	        return output;
	}

	void set_output(Wire w) {
		if (wire_names.count(w.get_name()) == 0) {
			throw UndefinedVariableError(w.get_name());
		}
		outputs.emplace_back(w);
	}

	const AssignmentList& get_assignments() const {
		return assignments;
	}

	const WireList& get_inputs() const {
		return inputs;
	}

  	const WireList& get_const_inputs() const {
		return const_inputs;
	}

	const WireList& get_wires() const {
		return wires;
	}

	const WireList& get_outputs() const {
		return outputs;
	}
};

std::ostream& operator<<(std::ostream& ostream, const Circuit& c);
std::istream& operator>>(std::istream& istream, Circuit& c);

bool operator==(const Circuit&, const Circuit&);
bool operator!=(const Circuit&, const Circuit&);

#endif // define CIRCUIT_HPP
