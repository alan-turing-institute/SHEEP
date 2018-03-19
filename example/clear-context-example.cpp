#include <memory>

#include "context-clear.hpp"

#include "circuit-repo.hpp"


using namespace Sheep::Clear;

int main(void) {

  typedef ContextClear<int8_t>::Plaintext Plaintext;
  typedef std::vector< Plaintext > PlaintextVec;  
  typedef ContextClear<int8_t>::Ciphertext Ciphertext;  
  
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

  

  PlaintextVec input0 = { 6, 7, 8, 9, 10, 11 };
  PlaintextVec input1 = { 9, 9, 9, 9, 9, 9 };
  PlaintextVec input2 = { 25, 25, 25, 25, 25, 25 };
  PlaintextVec input3 = { 67, 66, 65, 64, 63, 62 };
  

  std::vector< PlaintextVec > plaintext_inputs = {input0, input1, input2,input3};
  std::list<Ciphertext> ciphertext_inputs;
  
  for (PlaintextVec pt: plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));
  
  std::list<Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
  
  std::vector<PlaintextVec> plaintext_outputs;
  for (Ciphertext ct: ciphertext_outputs) {
    PlaintextVec pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << "output: "
	      << std::to_string(pt[0]) << " "
	      << std::to_string(pt[1]) <<" "
	      << std::to_string(pt[2]) <<" "
	      << std::to_string(pt[3]) <<" "
	      << std::to_string(pt[4]) <<" "      
	      <<  std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
  
}
