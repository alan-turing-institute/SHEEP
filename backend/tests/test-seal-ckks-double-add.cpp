#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-seal-ckks.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextSealCKKS<double> ctx;

  std::vector<std::vector<ContextSealCKKS<double>::Plaintext>> pt_input = {
      {15.1, 10.2, 100.3, -80.4}, {22.1, -12.2, 100.3, -80.4}};

  std::vector<std::vector<ContextSealCKKS<double>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input);

  std::vector<double> exp_values = {37.2, -2.0, 200.6, -160.8};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << std::to_string(pt_input[0][i]) << " + "
              << std::to_string(pt_input[1][i]) << " = "
              << std::to_string(result[0][i]) << std::endl;
    assert(abs(result.front()[i] -exp_values[i])<0.1);
  }

}
