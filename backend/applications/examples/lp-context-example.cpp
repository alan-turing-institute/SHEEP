#include <memory>

#include "context-lp.hpp"

#include "circuit-repo.hpp"

using namespace SHEEP;

int main(void) {
  //// instantiate the Circuit Repository
  CircuitRepo cr;

  ///  Circuit C = cr.get_circuit_by_name("TestCircuit1");

  Circuit c2 = cr.create_circuit(Gate::Add, 3);

  std::cout << c2;

  ContextLP<uint8_t> ctx;

  std::cout << "about to read params" << std::endl;

  // ctx.read_params_from_file("params.txt");

  // ctx.set_parameter("p",long(257));

  // ctx.print_parameters();

  // ContextLP<uint8_t>::CircuitEvaluator run_circuit;
  // run_circuit = ctx.compile(c2);

  std::list<ContextLP<uint8_t>::Plaintext> plaintext_inputs = {6, 9, 25, 67};
  std::list<ContextLP<uint8_t>::Ciphertext> ciphertext_inputs;

  for (ContextLP<uint8_t>::Plaintext pt : plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));

  std::list<ContextLP<uint8_t>::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = ctx.eval(c2, ciphertext_inputs, ciphertext_outputs);

  std::list<ContextLP<uint8_t>::Plaintext> plaintext_outputs;
  for (ContextLP<uint8_t>::Ciphertext ct : ciphertext_outputs) {
    ContextLP<uint8_t>::Plaintext pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << std::to_string(pt) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
}
