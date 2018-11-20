#include "simple-circuits.hpp"
#include "circuit-util.hpp"

Circuit single_unary_gate_circuit(Gate g) {
  Circuit C;
  Wire in = C.add_input("in");
  Wire out = C.add_assignment("out", g, in);
  C.set_output(out);
  return C;
}

Circuit single_binary_gate_circuit(Gate g) {
  Circuit C;
  Wire in1 = C.add_input("in1");
  Wire in2 = C.add_input("in2");
  Wire out = C.add_assignment("out", g, in1, in2);
  C.set_output(out);
  return C;
}

static Circuit bitonic_compare_and_swap(int n, bool up) {
  NameGenerator name;
  // Circuit with n inputs - most pass through
  // compare and swap the first and n/2'th elements
  Circuit C;
  for (int i = 0; i < n; i++) {
    Wire tmp = C.add_input(name("in"));
  }

  int offset = n / 2;
  const Circuit::WireList& input = C.get_inputs();
  Circuit::WireList second_half;
  for (int i = 0; i < offset; i++) {
    Wire a = input[i];
    Wire b = input[i + offset];
    Wire c = C.add_assignment(name("c"), Gate::Compare, a, b);
    Wire x(""), y("");
    if (up) {
      x = C.add_assignment(name("x"), Gate::Select, c, a, b);
      y = C.add_assignment(name("y"), Gate::Select, c, b, a);
    } else {
      x = C.add_assignment(name("x"), Gate::Select, c, b, a);
      y = C.add_assignment(name("y"), Gate::Select, c, a, b);
    }
    // Set the outputs of the circuit.  Can't immediately
    // set y as an output, since these should come after
    // all of the x's
    C.set_output(x);
    second_half.push_back(y);
  }
  for (auto x : second_half) C.set_output(x);

  return C;
}

static Circuit bitonic_merge(int n, bool up) {
  if (n == 1) {
    return single_unary_gate_circuit(Gate::Alias);
  } else {
    Circuit first = bitonic_merge(n / 2, up);
    Circuit second = bitonic_merge(n / 2, up);
    return seq(bitonic_compare_and_swap(n, up), par(first, second));
  }
}

// returns the bitonic sorting circuit of n items
// Assumption: n is a power of two
Circuit bitonic_sort(int n, bool up) {
  if (n == 1) {
    return single_unary_gate_circuit(Gate::Alias);
  } else {
    Circuit first = bitonic_sort(n / 2, true);
    Circuit second = bitonic_sort(n / 2, false);
    return seq(par(first, second), bitonic_merge(n, up));
  }
}
