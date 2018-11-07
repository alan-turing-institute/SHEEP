#include <memory>

#include "context-clear.hpp"

#include "circuit-repo.hpp"

using namespace SHEEP;

int main(void) {
  //// instantiate the Circuit Repository
  CircuitRepo cr;

  //// build a circuit with a specified depth of a specified gate

  Circuit C = cr.create_circuit(Gate::Add, 3);
  std::cout << C;

  ContextClear<int8_t> ctx;

  // ContextClear<int8_t>::CircuitEvaluator run_circuit;
  // run_circuit = ctx.compile(C);

  std::list<ContextClear<int8_t>::Plaintext> plaintext_inputs = {6, 9, 25, 67};
  std::list<ContextClear<int8_t>::Ciphertext> ciphertext_inputs;

  for (ContextClear<int8_t>::Plaintext pt : plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));

  std::list<ContextClear<int8_t>::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = ctx.eval(C, ciphertext_inputs, ciphertext_outputs);

  std::list<ContextClear<int8_t>::Plaintext> plaintext_outputs;
  for (ContextClear<int8_t>::Ciphertext ct : ciphertext_outputs) {
    ContextClear<int8_t>::Plaintext pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << "output: " << std::to_string(pt) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";

  if (plaintext_outputs.front() == 107) return 0;
  return -1;
}
