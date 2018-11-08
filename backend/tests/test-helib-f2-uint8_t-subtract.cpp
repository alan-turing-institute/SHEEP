#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Subtract, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_F2<uint8_t> ctx;

  std::vector<std::vector<ContextHElib_F2<uint8_t>::Plaintext>> pt_input = {
      {22, 10, 255, 0}, {15, 12, 255, 255}};

  std::vector<std::vector<ContextHElib_F2<uint8_t>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input, durations);

  std::vector<uint8_t> exp_values = {7, 254, 0, 1};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " - "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
