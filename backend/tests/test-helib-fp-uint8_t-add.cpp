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

  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextHElib_Fp<uint8_t> ctx;

  std::vector<std::vector<ContextHElib_Fp<uint8_t>::Plaintext>> pt_input = {
      {15, 100}, {22, 20}};

  std::vector<std::vector<ContextHElib_Fp<uint8_t>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input);

  std::vector<uint8_t> exp_values = {37, 120};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " + "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result.front()[i] == exp_values[i]);
  }
}
