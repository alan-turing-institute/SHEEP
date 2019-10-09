#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-seal-ckks.hpp"
#include "circuit-repo.hpp"
#include "simple-circuits.hpp"
#include <complex>
#include <cmath>

using namespace std::complex_literals;

using namespace SHEEP;
typedef ContextSealCKKS<std::complex<double> >::Plaintext Plaintext;
typedef ContextSealCKKS<std::complex<double> >::Ciphertext Ciphertext;

typedef std::chrono::duration<double, std::micro> DurationT;

int main() {
  //// instantiate the Circuit Repository
  CircuitRepo cr;

  Circuit circ = cr.create_circuit(Gate::Add, 1);
  std::cout << circ;
  std::vector<DurationT> durations;
  ContextSealCKKS<std::complex<double>> ctx;

  std::vector<std::vector<ContextSealCKKS<std::complex<double>>::Plaintext>> pt_input = {
    {15.1+0.1i, 10.2 + 4.4i}, {1.2 - 0.1i, 10.2 + 5.5i}
  };

  std::vector<std::vector<ContextSealCKKS<std::complex<double>>::Plaintext>> result =
      ctx.eval_with_plaintexts(circ, pt_input);

  std::vector<std::complex<double>> exp_values = {16.3, 20.4+9.9i};

  for (int i = 0; i < exp_values.size(); i++) {
    std::cout << pt_input[0][i] << " + "
              << pt_input[1][i] << " = "
              << result[0][i] << std::endl;
    assert(abs(result.front()[i] - exp_values[i])<0.1);
  }

}
