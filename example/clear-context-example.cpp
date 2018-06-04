#include <memory>

#include "context-clear.hpp"

#include "circuit-repo.hpp"


using namespace SHEEP;

int main(void) {

  
    //// instantiate the Circuit Repository
  CircuitRepo cr;


  /// can either retrieve pre-build test circuits by name:
  
  Circuit C = cr.get_circuit_by_name("TestCircuit1");
  std::cout << C;

  //// or build a circuit with a specified depth of a specified gate
  
  // Circuit C2 = cr.create_circuit(Gate::Add, 3);
  // std::cout << C2;
  
  
  ContextClear<int8_t> ctx;
  
  ContextClear<int8_t>::CircuitEvaluator run_circuit;
  run_circuit = ctx.compile(C);
	
  std::list<ContextClear<int8_t>::Plaintext> plaintext_inputs = {6, 9, 25,67};
  std::list<ContextClear<int8_t>::Ciphertext> ciphertext_inputs;
  
  for (ContextClear<int8_t>::Plaintext pt: plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));
  
  std::list<ContextClear<int8_t>::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
  
  std::list<ContextClear<int8_t>::Plaintext> plaintext_outputs;
  for (ContextClear<int8_t>::Ciphertext ct: ciphertext_outputs) {
    ContextClear<int8_t>::Plaintext pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << "output: "<<std::to_string(pt) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
  
}
