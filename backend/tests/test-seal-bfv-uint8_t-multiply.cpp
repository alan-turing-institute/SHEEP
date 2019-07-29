#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-seal-bfv.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Multiply, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextSealBFV<uint8_t> ctx;

  // ContextSealBFV<uint8_t>::CircuitEvaluator run_circuit;
  // run_circuit = ctx.compile(circ);

  /// test small postitive numbers
  std::vector<std::vector<uint8_t> > inputs{{5}, {22}};
  std::vector<std::vector<uint8_t> > result =
      ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 5*22 = " << std::to_string(result.front()[0]) << std::endl;
  assert(result.front()[0] == 110);

  /// test result going out of range positive
  inputs = {{100}, {127}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 100 + 127 = " << std::to_string(result.front()[0])
            << std::endl;
  assert(result.front()[0] == 156);

  /// test slots
  /// test slots
  inputs = {{10, 11, 12}, {11, 12, 12}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  std::cout << "(10,11,12) * (11,12,12) == (" << std::to_string(result[0][0])
            << "," << std::to_string(result[0][1]) << ","
            << std::to_string(result[0][2]) << ")\n";
  assert(result[0][0] == 110);
  assert(result[0][1] == 132);
  assert(result[0][2] == 144);
}
