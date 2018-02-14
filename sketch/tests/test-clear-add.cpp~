#include <memory>

#include "context-clear.hpp"

#include "circuit-repo.hpp"

int main(void) {

  
    //// instantiate the Circuit Repository
  CircuitRepo cr;


  /// can either retrieve pre-build test circuits by name:
  
  Circuit C = cr.get_circuit_by_name("TestCircuit1");
  std::cout << C;

  //// or build a circuit with a specified depth of a specified gate
  
  // Circuit C2 = cr.create_circuit(Gate::Add, 3);
  // std::cout << C2;
  
  
  ContextClear ctx;
  
  ContextClear::CircuitEvaluator run_circuit;
  run_circuit = ctx.compile(C);
	
  std::list<ContextClear::Plaintext> plaintext_inputs = {6, 9, 25,67};
  std::list<ContextClear::Ciphertext> ciphertext_inputs;
  
  for (ContextClear::Plaintext pt: plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));
  
  std::list<ContextClear::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
  
  std::list<ContextClear::Plaintext> plaintext_outputs;
  for (ContextClear::Ciphertext ct: ciphertext_outputs) {
    ContextClear::Plaintext pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << "output: "<<std::to_string(pt) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
  
}
