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

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextSealBFV<int8_t> ctx;

  /// test small postitive numbers
  std::vector<std::vector<int8_t> > inputs = {{15}, {22}};
  std::vector<std::vector<int8_t> > result =
      ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 15+22 = " << std::to_string(result.front()[0]) << std::endl;
  assert(result.front()[0] == 37);

  /// test small negative numbers
  inputs = {{10}, {-12}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 10 - 12 = " << std::to_string(result.front()[0]) << std::endl;
  assert(result.front()[0] == -2);

  /// test result going out of range positive
  inputs = {{100}, {127}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  std::cout << " 100 + 127 = " << std::to_string(result.front()[0])
            << std::endl;
  assert(result.front()[0] == -29);

  /// test result going out of range negative
  inputs = {{-120}, {-124}};
  std::cout << " -120 - 124 = " << std::to_string(result.front()[0])
            << std::endl;
  result = ctx.eval_with_plaintexts(circ, inputs);
  assert(result.front()[0] == 12);

  /// test slots
  inputs = {{15, 1, -1}, {22, -2, 1}};
  result = ctx.eval_with_plaintexts(circ, inputs);
  assert(result[0][0] == 37);
  assert(result[0][1] == -1);
  assert(result[0][2] == 0);
}
