#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-lp.hpp"
#include "simple-circuits.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextLP<bool> ctx;
  ctx.set_parameter("NumSlots", 5);
  std::vector<std::vector<ContextLP<bool>::Plaintext>> pt_input = {
      {1, 0, 1, 0, 1}, {0, 0, 1, 1, 0}};

  std::vector<std::vector<ContextLP<bool>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input, durations);

  std::vector<bool> exp_values = {1, 0, 0, 1, 1};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " + "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
  }
  std::cout << " lengths of vectors " << result.front().size() << " "
            << exp_values.size() << std::endl;
  assert(result.front() == exp_values);
}
