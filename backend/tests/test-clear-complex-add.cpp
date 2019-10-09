#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-clear.hpp"

#include <complex>
#include <cmath>

using namespace std::complex_literals;

typedef std::chrono::duration<double, std::micro> DurationT;

int main(void) {
  using namespace SHEEP;

  //// instantiate the Circuit Repository
  CircuitRepo cr;

  // circ contains an Add gate with two inputs and one output
  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;


  ContextClear<std::complex<double>> ctx;

  // test small postitive numbers
  std::vector<std::vector<std::complex<double>>> inputs = {{1.5+1.5i}, {22.2-1.0i}};
  std::vector<std::complex<double>> exp_values = {23.7+0.5i};

  std::vector<std::vector<std::complex<double>>> result =
      ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << inputs[0][i] << " + "
              << inputs[1][i] << " = "
              << result[0][i] << std::endl;
    assert(abs(result.front()[i] - exp_values[i])<0.1);
  }

  // test positive number arrays
  inputs = {{41.1, 5.2+1.0i, 100.0-4.4i}, {40.1, 4.2, 111.0}};
  exp_values = {81.2, 9.4+1.0i, 211.0-4.4i};

  result = ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << inputs[0][i] << " + "
              << inputs[1][i] << " = "
              << result[0][i] << std::endl;
    assert(abs(result.front()[i] - exp_values[i])<0.1);
  }
}
