#include <memory>

#include "context-helib.hpp"

#include "circuit-repo.hpp"

using namespace Sheep::HElib;

typedef ContextHElib_F2<int8_t>::Plaintext Plaintext;
typedef std::vector< Plaintext > PlaintextVec;  
typedef ContextHElib_F2<int8_t>::Ciphertext Ciphertext;  

int main(void) {
  //// instantiate the Circuit Repository
  CircuitRepo cr;
  
  ///  Circuit C = cr.get_circuit_by_name("TestCircuit1");
  

  Circuit c2 = cr.create_circuit(Gate::Add, 1);

  std::cout << c2;

  
  ContextHElib_F2<int8_t> ctx;

  ///  std::cout<<"about to read params"<<std::endl;
  
  ///ctx.read_params_from_file("params.txt");
  
  ///  ctx.set_parameter("p",long(257));
  
  ContextHElib_F2<int8_t>::CircuitEvaluator run_circuit;
  run_circuit = ctx.compile(c2);

  PlaintextVec input0 = { -12, -128, -128, -128, 127, 127, 127, 127, 0, 0, 0, 0, -51, -51, -51, -51 };
  PlaintextVec input1 = { -128, 127, 0, -51, -128, 127, 0, -51, -128, 127, 0, -51, -128, 127, 0, -51 };
  

  std::vector< PlaintextVec > plaintext_inputs = {input0, input1};
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
    std::cout << "output: ";
    for (int i = 0; i< pt.size(); i++) {
      std::cout<< std::to_string(pt[i]) << " ";
    }
    std::cout <<  std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";

}
