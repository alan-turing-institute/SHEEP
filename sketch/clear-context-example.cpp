#include <memory>

#include "context-clear.hpp"

#include "circuit-repo.hpp"

int main(void) {

  
    //// instantiate the Circuit Repository
  CircuitRepo cr;
  
  //Circuit C = cr.get_test_circuit_1();

  //  Circuit C = cr.get_circuit_by_name("TestCircuit1");

  // C.print();


  Circuit C2 = cr.create_circuit(Gate::And, 3);
  C2.print();
  
  
  
  ContextClear ctx;
  
  ContextClear::CircuitEvaluator run_circuit;
  run_circuit = ctx.compile(C2);
	
  std::list<ContextClear::Plaintext> plaintext_inputs = {true, false, true, false};
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
    std::cout << pt << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
  
}
