#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-seal.hpp"

int main(void) {
  using namespace SHEEP;

  // repeat the job 100 times for statistics
  for (int j = 0; j < 100; j++) {

    Circuit circ;
    ContextSeal<bool> ctx;

    std::vector<std::vector<bool>> pt_input;
    std::vector<std::vector<bool>> result;
    std::vector<bool> exp_values;

    // The circuit
    Wire a = circ.add_input("a");
    Wire b = circ.add_input("b");
    Wire c = circ.add_input("c");
    Wire d = circ.add_input("d");
    Wire e = circ.add_input("e");
    Wire w2 = circ.add_assignment("w2", Gate::Add, a, b);
    Wire w3 = circ.add_assignment("w3", Gate::Multiply, w2, c);
    Wire w4 = circ.add_assignment("w4", Gate::Add, w3, d);
    Wire w5 = circ.add_assignment("w5", Gate::Multiply, w2, e);
    Wire w6 = circ.add_assignment("w6", Gate::Add, w4, w5);
    Wire w7 = circ.add_assignment("w7", Gate::Add, w3, w6);
    Wire w8 = circ.add_assignment("w8", Gate::Multiply, w7, w6);

    circ.set_output(w8);

    pt_input = {{0}, {1}, {0}, {1}, {0}};
    result = ctx.eval_with_plaintexts(circ, pt_input);

    exp_values = {1};

    for (int i = 0; i < exp_values.size(); i++) {

      std::cout 
        << std::to_string(i) << " | " 
        << std::to_string(pt_input[0][i]) << " , " 
        << std::to_string(pt_input[1][i]) << " , " 
        << std::to_string(pt_input[2][i]) << " , " 
        << std::to_string(pt_input[3][i]) << " , " 
        << std::to_string(pt_input[4][i]) << " = " 
        << std::to_string(result[0][i]) << " | " 
        << std::to_string(exp_values[i]) << std::endl;

      assert(result.front()[i] == exp_values[i]);
    }
  }
}
