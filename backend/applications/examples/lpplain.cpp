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


  std::vector< std::vector<ContextLP<uint8_t>::Plaintext> > plaintext_inputs = {{6}, {9}, {25}, {67}};
  std::list<ContextLP<uint8_t>::Ciphertext> ciphertext_inputs;

  for (std::vector<ContextLP<uint8_t>::Plaintext> pt : plaintext_inputs)
    ciphertext_inputs.push_back(ctx.MultByConstant(ctx.encrypt(pt), 2));

  std::vector<ContextLP<uint8_t>::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = ctx.eval(c2, ciphertext_inputs, ciphertext_outputs);

  std::vector<std::vector<ContextLP<uint8_t>::Plaintext> > plaintext_outputs;
  for (ContextLP<uint8_t>::Ciphertext ct : ciphertext_outputs) {
    std::vector<ContextLP<uint8_t>::Plaintext> pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << std::to_string(pt[0]) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
}
