#include <memory>
#include <fstream>
#include <map>
#include "context-clear.hpp"


using namespace Sheep;

int main(int argc, const char** argv) {


  //  typename PlaintextType;
  // typename CiphertextType;
  // std::map<std::string, Context<PlaintextType, CiphertextType>* > context_map;

  
  if (argc < 2) return -1;
  std::ifstream input_file(argv[1]);
  
    
  Circuit C;

  input_file >> C;

  std::cout << C;

  if (argc < 3) return -2;

  int8_t input_val = atoi(argv[2]);
  
  Clear::ContextClear<int8_t> ctx;


  Clear::ContextClear<int8_t>::CircuitEvaluator run_circuit;
  run_circuit = ctx.compile(C);
	
  
  std::list<Clear::ContextClear<int8_t>::Plaintext> plaintext_inputs = {input_val};
  std::list<Clear::ContextClear<int8_t>::Ciphertext> ciphertext_inputs;
  
  for (Clear::ContextClear<int8_t>::Plaintext pt: plaintext_inputs)
    ciphertext_inputs.push_back(ctx.encrypt(pt));
  
  std::list<Clear::ContextClear<int8_t>::Ciphertext> ciphertext_outputs;
  using microsecond = std::chrono::duration<double, std::micro>;
  microsecond time = run_circuit(ciphertext_inputs, ciphertext_outputs);
  
  std::list<Clear::ContextClear<int8_t>::Plaintext> plaintext_outputs;
  for (Clear::ContextClear<int8_t>::Ciphertext ct: ciphertext_outputs) {
    Clear::ContextClear<int8_t>::Plaintext pt = ctx.decrypt(ct);
    plaintext_outputs.push_back(pt);
    std::cout << "output: "<<std::to_string(pt) << std::endl;
  }
  std::cout << "time was " << time.count() << " microseconds\n";
  
  
}
