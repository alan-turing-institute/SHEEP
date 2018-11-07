#include <cassert>
#include <fstream>
#include "circuit.hpp"

int main(void) {
  std::ifstream good("tests/good.sheep");
  assert(good.is_open());
  Circuit C_good;
  good >> C_good;

  std::ifstream multiple_input("tests/multiple-input.sheep");
  assert(multiple_input.is_open());
  Circuit C_multiple_input;
  try {
    multiple_input >> C_multiple_input;
  } catch (const MultipleAssignmentError& e) {
    std::cout << "'multiple-input.sheep': Caught MultipleAssignmentError (ok)\n"
              << "   What: " << e.what() << std::endl;
  }

  std::ifstream multiple_assignment("tests/multiple-assignment.sheep");
  assert(multiple_assignment.is_open());
  Circuit C_multiple_assignment;
  try {
    multiple_assignment >> C_multiple_assignment;
  } catch (const MultipleAssignmentError& e) {
    std::cout
        << "'multiple-assignment.sheep': Caught MultipleAssignmentError (ok).\n"
        << "   What: " << e.what() << std::endl;
  }

  std::ifstream undef_gate_input("tests/undef-gate-input.sheep");
  assert(undef_gate_input.is_open());
  Circuit C_undef_gate_input;
  try {
    undef_gate_input >> C_undef_gate_input;
  } catch (const UndefinedVariableError& e) {
    std::cout << "'undef-gate-input.sheep': Caught UnusedVariableError (ok).\n"
              << "   What: " << e.what() << std::endl;
  }

  std::ifstream undef_output("tests/undef-output.sheep");
  assert(undef_output.is_open());
  Circuit C_undef_output;
  try {
    undef_output >> C_undef_output;
  } catch (const UndefinedVariableError& e) {
    std::cout << "'undef-output.sheep': Caught UnusedVariableError (ok).\n"
              << "   What: " << e.what() << std::endl;
  }

  return 0;
}
