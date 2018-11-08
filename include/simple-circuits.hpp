#ifndef SIMPLE_CIRCUITS_HPP
#define SIMPLE_CIRCUITS_HPP

#include "circuit.hpp"

Circuit single_unary_gate_circuit(Gate g);
Circuit single_binary_gate_circuit(Gate g);
Circuit bitonic_sort(int n, bool up);

#endif  // SIMPLE_CIRCUITS_HPP
