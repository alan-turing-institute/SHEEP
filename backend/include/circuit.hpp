/*
 * == Circuits and circuit elements ==
 *
 * A circuit (class Circuit) is a directed acyclic graph of circuit
 * elements, which is the representation of a computation used by the
 * sheep evaluator.  These circuits are 'arithmetic' rather than
 * logical, and this is reflected in the language used (e.g. we have a
 * gate called 'Multiply' rather than 'And').
 *
 * The circuit is a completely abstract representation of the
 * computation to be performed, and the 'type' over which circuits
 * compute is deliberately not specified by this part of the library.
 * The actual computation (and therefore the type of the values being
 * computed on) is provided by the Context class: refer to the
 * description in context.hpp for how the evaluation of a circuit is
 * performed.  In general it will be over however the library
 * represented ciphertext values (but see below about 'constant' or
 * 'plaintext' inputs).
 *
 * A circuit element is represented by a Gate.  This is an enum, and
 * so has no structure itself (again, their interpretation is provided
 * by a Context class).  The Gates are connected by 'Wires': a Gate
 * along with its input and output Wires are contained in an
 * 'Assignment' class.  A Wire is little more than a name stored as a
 * string.  Finally, the representation of a circuit in the Circuit
 * class is provided by a collection of Assignments.
 *
 * Assignments can have arbitrary fan-in but always have fan-out of
 * one.  Gates with fan-out > 1 (e.g. Swap) may be supported in
 * future.
 *
 * Note that _circuit inputs_ and _assignment inputs_ are different
 * concepts (a gate input is not necessarily a circuit input; a
 * circuit input is in general the input to at least one gate, but it
 * might not be).
 *
 * Wires are compared by their name (of type std::string). This may
 * seem inefficient, but adds negligable overhead to the calculation
 * of the homomorphic operations, which are normally a lot more
 * expensive.
 *
 * Any computation performed by SHEEP is represented as a circuit.  It
 * is maximal for what can be computed by SHEEP: that is, not all of
 * the libraries, or their representation as SHEEP Context types, need
 * to support all of the Gates or any conceivable circuit.  Anything
 * that a library supports that we want to expose should be
 * representable in this form however: this might sometimes
 * necessitate adding a new operation.
 *
 * == Inputs and Constant/Plaintext Inputs ==
 *
 * Most HE libraries support a limited number of operations where one
 * operand is a ciphertext value and the other is a plaintext value.
 * For example, MultByConstant takes a Ciphertext and a Plaintext and
 * returns their product.  The plaintexts for these operations are
 * provided to the circuit as CONST_INPUTS in the circuit input file.
 *
 */

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// An enum representing the arithmetic Gates supported by a SHEEP
// circuit.  Their meaning is documented in context.hpp.
enum class Gate {
  Alias,
  Identity,
  Multiply,
  Maximum,
  Add,
  Subtract,
  Negate,
  Compare,
  Select,
  AddConstant,
  MultByConstant,
  Rotate
};

// This is used by the interpreter for the parser of the SHEEP
// language to map the names of operations to Gates as defined above.
static const std::map<std::string, Gate> gate_name_map = {
    {"ALIAS", Gate::Alias},
    {"ID", Gate::Identity},
    {"ADD", Gate::Add},
    {"MULTIPLY", Gate::Multiply},
    {"SUBTRACT", Gate::Subtract},
    {"MAXIMUM", Gate::Maximum},
    {"NEGATE", Gate::Negate},
    {"COMPARE", Gate::Compare},
    {"SELECT", Gate::Select},
    {"ADDCONST", Gate::AddConstant},
    {"MULTBYCONST", Gate::MultByConstant},
    {"ROTATE", Gate::Rotate}};

// Wires - identified by their name.
class Wire {
  std::string name;

 public:
  Wire(std::string name_) : name(name_) {}
  const std::string get_name() const { return name; }
};

bool operator==(const Wire&, const Wire&);
bool operator!=(const Wire&, const Wire&);

// An 'assignment' in the circuit, which comprises a list of Wires
// that hold the input values to the operation, a list of Wires that
// hold the plaintext inputs ('const_inputs') to the operation, a
// single output wire and the operation itself (of type Gate).
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

  size_t input_count() const { return inputs.size(); }
  const WireList& get_inputs() const { return inputs; }
  const WireList& get_const_inputs() const { return const_inputs; }
  Wire get_output() const { return output; }
  Gate get_op() const { return op; }
};

template <typename... Ts>
Assignment::Assignment(Wire output_, Gate op_, Ts... inputs_)
    : output(output_), op(op_), inputs{inputs_...} {}

template <>
Assignment::Assignment(Wire, Gate, Assignment::WireList);

bool operator==(const Assignment&, const Assignment&);
bool operator!=(const Assignment&, const Assignment&);

struct MultipleAssignmentError : public std::runtime_error {
  const std::string name;
  MultipleAssignmentError(std::string name_)
      : name(name_),
        std::runtime_error("Multiple assignment to wire named '" + name_ +
                           "'"){};
};

struct UndefinedVariableError : public std::runtime_error {
  const std::string name;
  UndefinedVariableError(std::string name_)
      : name(name_),
        std::runtime_error("A wire with name '" + name_ +
                           "' has not yet been defined."){};
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

  // The input Wires to a Circuit
  WireList inputs;

  // The 'internal' Wires (those that are not inputs)
  WireList wires;

  // The output Wires of the Circuit
  WireList outputs;

  // The Wires representing the plaintext inputs of the Circuit
  WireList const_inputs;

  // The Assignments (stored in topological order)
  AssignmentList assignments;

  // Try to insert a unique wire name into the set of wires.  Throw
  // a MultipleAssignmentError if this already exists.
  void try_insert_wire_name(const std::string& name) {
    // std::unordered_set::insert returns an iterator pointing to the
    // element that was inserted (it_ignored) and a boolean indicating
    // whether the insertion took place.  If it did not, then the wire
    // was already present in the list of wires.  This means that this
    // was a multiple assignment, which is an error.
    std::unordered_set<std::string>::iterator it_ignored;
    bool inserted;
    std::tie(it_ignored, inserted) = wire_names.insert(name);
    if (!inserted) throw MultipleAssignmentError(name);
  }

 public:
  Wire add_input(std::string name) {
    try_insert_wire_name(name);
    inputs.emplace_back(name);
    return inputs.back();
  }

  Wire add_const_input(std::string name) {
    try_insert_wire_name(name);
    const_inputs.emplace_back(name);
    return const_inputs.back();
  }

  // Add an Assignment to the Circuit.  Assignments must be added in
  // topological order (that is, an assignment must only use wires
  // that are already present in the circuit for input).
  template <typename... Wires>
  Wire add_assignment(std::string name, Gate op, Wires... ws) {
    try_insert_wire_name(name);
    wires.emplace_back(name);

    // this Wire will be the output of the Assignment we are about to
    // construct
    Wire output = wires.back();

    Assignment assgn(output, op, ws...);

    // Check that each named input into the Assignment assgn exists in
    // the Circuit (by querying the count of it in the set of wire
    // names.  If not, we have tried to use a variable before it is
    // defined.
    for (auto assgn_in : assgn.get_inputs()) {
      if (wire_names.count(assgn_in.get_name()) == 0) {
        throw UndefinedVariableError(assgn_in.get_name());
      }
    }

    assignments.push_back(std::move(assgn));

    return output;
  }

  // Nominate an existing Wire as an output of the Circuit.
  void set_output(Wire w) {
    if (wire_names.count(w.get_name()) == 0) {
      throw UndefinedVariableError(w.get_name());
    }
    outputs.emplace_back(w);
  }

  const AssignmentList& get_assignments() const { return assignments; }

  const WireList& get_inputs() const { return inputs; }

  const WireList& get_const_inputs() const { return const_inputs; }

  const WireList& get_wires() const { return wires; }

  const WireList& get_outputs() const { return outputs; }
};

// The stream operators can be used to read and write Circuits in the
// SHEEP language format.
std::ostream& operator<<(std::ostream& ostream, const Circuit& c);
std::istream& operator>>(std::istream& istream, Circuit& c);

// Circuit (in)equality.  Two circuits are defined to be equal if
// their input wires, output wires and assignments all compare equal,
// when taken in their storage order in the respective WireList and
// AssignmentLits.  This is useful mostly for testing purposes.
//
// This is a shallow comparison: two circuits could be topologically
// equivalent but compare false if e.g. two assignments are specified
// in a different order, or if two equivalent circuits have some wires
// (consistently) renamed.  Two circuits could be functionally
// equivalent with a different number of assignments and these would
// also compare false.
bool operator==(const Circuit&, const Circuit&);
bool operator!=(const Circuit&, const Circuit&);

#endif  // define CIRCUIT_HPP
