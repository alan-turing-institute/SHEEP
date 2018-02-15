#include "simple-circuits.hpp"

Circuit single_binary_gate_circuit(Gate g) {
	Circuit C;
	Wire in1 = C.add_input("in1");
	Wire in2 = C.add_input("in2");
	Wire out = C.add_assignment("out", g, in1, in2);
	C.set_output(out);
	return C;
}
