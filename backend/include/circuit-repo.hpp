#ifndef CIRCUIT_REPO_HPP
#define CIRCUIT_REPO_HPP

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "circuit.hpp"

///// repository of pre-built circuits, and ability to specify circuits of given
///depth with

class CircuitRepo {
 public:
  CircuitRepo() {
    build_test_circuit_1();
    build_test_circuit_2();
    build_test_circuit_3();
  }

  void build_test_circuit_1() {
    Circuit C;
    Wire a = C.add_input("a");
    Wire b = C.add_input("b");
    Wire c = C.add_input("c");
    Wire d = C.add_input("d");
    Wire w2 = C.add_assignment("w2", Gate::Add, a, b);
    Wire w3 = C.add_assignment("w3", Gate::Multiply, w2, c);

    C.set_output(w3);
    C.set_output(d);

    m_circuitStore.insert({"TestCircuit1", C});
  }

  void build_test_circuit_2() {
    Circuit C;
    Wire a = C.add_input("a");
    Wire b = C.add_input("b");
    Wire c = C.add_input("c");
    Wire d = C.add_input("d");
    Wire w2 = C.add_assignment("w2", Gate::Multiply, a, b);
    Wire w3 = C.add_assignment("w3", Gate::Multiply, w2, c);
    Wire w4 = C.add_assignment("w4", Gate::Subtract, w3, d);

    C.set_output(w4);

    m_circuitStore.insert({"TestCircuit2", C});
  }

  void build_test_circuit_3() {
    Circuit C;
    Wire a = C.add_input("a");
    Wire b = C.add_input("b");
    Wire c = C.add_input("c");
    Wire d = C.add_input("d");
    Wire e = C.add_input("e");
    Wire w2 = C.add_assignment("w2", Gate::Add, a, b);
    Wire w3 = C.add_assignment("w3", Gate::Multiply, w2, c);
    Wire w4 = C.add_assignment("w4", Gate::Subtract, w3, d);
    Wire w5 = C.add_assignment("w5", Gate::Negate, e);
    Wire w6 = C.add_assignment("w6", Gate::Maximum, w4, w5);

    C.set_output(w6);

    m_circuitStore.insert({"TestCircuit3", C});
  }

  Circuit create_circuit(Gate gate, int depth) {
    Circuit C;

    std::vector<Wire> input_wires;
    std::vector<Wire> output_wires;
    std::string input_prefix = "input_";
    std::string output_prefix = "output_";

    if (gate == Gate::Negate) {
      /// one input and one output per gate
      Wire input_wire = C.add_input(input_prefix + "0");
      input_wires.push_back(input_wire);

      Wire gate_output = C.add_assignment("output_0", gate, input_wire);
      output_wires.push_back(gate_output);

      for (int i = 1; i < depth; ++i) {
        Wire gate_output = C.add_assignment("output_" + std::to_string(i), gate,
                                            output_wires[i - 1]);
        output_wires.push_back(gate_output);
      }

    } else if (gate == Gate::Select) {
      /// two input wires, one select wire,  and one output per gate
      std::vector<Wire> select_wires;
      for (int i = 0; i < depth + 1; ++i) {
        Wire input_wire = C.add_input(input_prefix + std::to_string(i));
        input_wires.push_back(input_wire);

        if (i < depth) {
          Wire select_wire = C.add_input("select_" + std::to_string(i));
          select_wires.push_back(select_wire);
        }
      }

      //// make the first output from the first two inputs and the first select
      if (input_wires.size() > 1) {
        Wire gate_output = C.add_assignment("output_0", gate, select_wires[0],
                                            input_wires[0], input_wires[1]);
        output_wires.push_back(gate_output);

        ///  loop over the rest of the inputs, combining an input with an output
        ///  from the previous operation and a select wire.
        for (unsigned long i = 2; i < input_wires.size(); ++i) {
          Wire gate_output = C.add_assignment(
              "output_" + std::to_string(i - 1), gate, select_wires[i - 1],
              output_wires[i - 2], input_wires[i]);
          output_wires.push_back(gate_output);
        }
      }

    } else {
      /// assume gates have two inputs and one output - we need depth+1 inputs
      for (int i = 0; i < depth + 1; ++i) {
        Wire input_wire = C.add_input(input_prefix + std::to_string(i));
        input_wires.push_back(input_wire);
      }

      //// make the first output from the first two inputs
      if (input_wires.size() > 1) {
        Wire gate_output =
            C.add_assignment("output_0", gate, input_wires[0], input_wires[1]);
        output_wires.push_back(gate_output);
      }

      ///  loop over the rest of the inputs, combining an input with an
      /// output from the previous operation..
      for (unsigned long i = 2; i < input_wires.size(); ++i) {
        Wire gate_output =
            C.add_assignment("output_" + std::to_string(i - 1), gate,
                             output_wires[i - 2], input_wires[i]);
        output_wires.push_back(gate_output);
      }
    }  //// end-of 2-to-1 circuits

    /// set the output of the circuit to be the last gate output wire.
    C.set_output(output_wires.back());
    return C;
  }

  const Circuit get_circuit_by_name(std::string name) {
    auto iter = m_circuitStore.find(name);
    if (iter != m_circuitStore.end())
      return iter->second;
    else {  /// return an empty circuit
      std::cout << name << " not found - returning an empty circuit"
                << std::endl;
      Circuit C;
      return C;
    }
  }

 private:
  std::unordered_map<std::string, const Circuit> m_circuitStore;
};

#endif  // define CIRCUIT_REPO_HPP
