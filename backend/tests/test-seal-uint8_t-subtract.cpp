#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-seal.hpp"
#include "simple-circuits.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Subtract, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextSeal<uint8_t> ctx;
  ctx.set_parameter("NumSlots", 4);
  std::vector<std::vector<ContextSeal<uint8_t>::Plaintext>> pt_input = {
      {22, 16, 100, 120}, {15, 12, 27, 0}};

  std::vector<std::vector<ContextSeal<uint8_t>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input);

  std::vector<uint8_t> exp_values = {7, 4, 73, 120};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " - "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(result[0][i] == exp_values[i]);
  }


}
