#include <memory>

#include "context-helib.hpp"

#include "circuit-repo.hpp"


int main(void) {
  //// instantiate the Circuit Repository
  CircuitRepo cr;
  
  Circuit C = cr.get_circuit_by_name("TestCircuit1");
  

  Circuit c2 = cr.create_circuit(Gate::Add, 3);

  std::cout << c2;

  
  ContextHElib ctx(20,80);
  
  ContextHElib::CircuitEvaluator run_circuit;
  run_circuit = ctx.compile(C);
	
  std::list<ContextHElib::Plaintext> plaintext_inputs = {6, 9, 25, 67};
  std::list<ContextHElib::Ciphertext> ciphertext_inputs;
  
  for (ContextHElib::Plaintext pt: plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));
  
  std::list<ContextHElib::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
  
  std::list<ContextHElib::Plaintext> plaintext_outputs;
  for (ContextHElib::Ciphertext ct: ciphertext_outputs) {
    ContextHElib::Plaintext pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << std::to_string(pt) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
}
