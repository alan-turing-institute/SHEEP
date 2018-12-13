#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-seal.hpp"

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextSeal<uint8_t> ctx;

  // ContextSeal<uint8_t>::CircuitEvaluator run_circuit;
  // run_circuit = ctx.compile(circ);

  /// test small postitive numbers
  std::vector<std::vector<uint8_t> > inputs{{5}, {22}};
  std::vector<std::vector<uint8_t> > result =
      ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 5+22 = " << std::to_string(result.front()[0]) << std::endl;
  assert(result.front()[0] == 27);

  /// test result going out of range positive
  inputs = {{200}, {127}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 200 + 127 = " << std::to_string(result.front()[0])
            << std::endl;
  assert(result.front()[0] == 71);

  /// test slots
  inputs = {{1, 1, 1}, {2, 3, 255}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  assert(result[0][0] == 3);
  assert(result[0][1] == 4);
  assert(result[0][2] == 0);
}
