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

  // circ contains an Multiply gate with two inputs and one output
  Circuit circ = cr.create_circuit(Gate::Multiply, 1);
  std::cout << circ;
  std::vector<DurationT> durations;


  ContextClear<std::complex<double>> ctx;

  // test small postitive numbers
  std::vector<std::vector<std::complex<double>>> inputs = {{1.5+1.5i}, {22.0-1.0i}};
  std::vector<std::complex<double>> exp_values = {34.5+31.5i};

  std::vector<std::vector<std::complex<double>>> result =
      ctx.eval_with_plaintexts(circ, inputs);

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << inputs[0][i] << " * "
              << inputs[1][i] << " = "
              << result[0][i] << std::endl;
    assert(abs(result.front()[i] - exp_values[i])<0.1);
  }

}
