#include "circuit.hpp"

static void test_good(void)
{
	Circuit C;
	Wire in1 = C.add_input("in1");
	Wire in2 = C.add_input("in2");
	Wire out = C.add_assignment("out", Gate::Add, std::string("in1"), std::string("in2"));
	C.set_output(out);
}

static void test_multiple_input(void)
{
	Circuit C;
	Wire in1 = C.add_input("reused_name");
	Wire in2 = C.add_input("reused_name");
	Wire out = C.add_assignment("out", Gate::Add, in1, in2);
	C.set_output(out);
}

static void test_multiple_assignment(void)
{
	Circuit C;
	Wire in1 = C.add_input("in1");
	Wire in2 = C.add_input("in2");
	Wire w1 = C.add_assignment("reused_name", Gate::Add, std::string("in1"), std::string("in2"));
	Wire w2 = C.add_assignment("reused_name", Gate::Add, std::string("in1"), std::string("in2"));
}

static void test_undef_gate_input(void)
{
	Circuit C;
	Wire in1 = C.add_input("in1");
	Wire in2 = C.add_input("in2");
	Wire out = C.add_assignment("out", Gate::Add, std::string("in1"), std::string("unused_name"));
	C.set_output(out);
}

static void test_undef_output(void)
{
	Circuit C;
	Wire in1 = C.add_input("in1");
	Wire in2 = C.add_input("in2");
	Wire out = C.add_assignment("out", Gate::Add, in1, in2);
	C.set_output(std::string("unused_name"));
}

int main(void)
{
	test_good();

	try {
		test_multiple_input();
	}
	catch (const MultipleAssignmentError&) {
		std::cout << "test_multiple_input: Caught MultipleAssignmentError (ok).\n";
	}

	try {
		test_multiple_assignment();
	}
	catch (const MultipleAssignmentError&) {
		std::cout << "test_multiple_assignment: Caught MultipleAssignmentError (ok).\n";
	}
	
	try {
		test_undef_gate_input();
	}
	catch (const UndefinedVariableError&) {
		std::cout << "test_undef_gate_input: Caught UnusedVariableError (ok).\n";
	}

	try {
		test_undef_output();
	}
	catch (const UndefinedVariableError&) {
		std::cout << "test_undef_output: Caught UnusedVariableError (ok).\n";
	}

	return 0;
}
