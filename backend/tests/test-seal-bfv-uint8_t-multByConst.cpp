#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-seal-bfv.hpp"

int main(void) {
  using namespace SHEEP;

  ContextSealBFV<uint8_t> ctx;

  /// test small postitive numbers
  std::vector<ContextSealBFV<uint8_t>::Plaintext> pt_input{55};
  ContextSealBFV<uint8_t>::Ciphertext ct = ctx.encrypt(pt_input);
  long const_val = 2;
  ContextSealBFV<uint8_t>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);
  std::vector<ContextSealBFV<uint8_t>::Plaintext> pt_out = ctx.decrypt(ct_out);
  std::cout << "D( E(55) * 2) = " << std::to_string(pt_out[0]) << std::endl;
  assert(pt_out[0] == 110);
  /// out-of-range positive
  pt_input[0] = 120;
  ct = ctx.encrypt(pt_input);
  const_val = 121;
  ct_out = ctx.MultByConstant(ct, const_val);
  pt_out = ctx.decrypt(ct_out);
  std::cout << "D( E(120) * 121) = " << std::to_string(pt_out[0]) << std::endl;
  assert(pt_out[0] == 184);

  /// test slots
  pt_input = {1, 60, 12};
  ct = ctx.encrypt(pt_input);
  const_val = 4;
  ct_out = ctx.MultByConstant(ct, const_val);
  pt_out = ctx.decrypt(ct_out);

  std::cout << "D[E(1,60,12) * 4] == (" << std::to_string(pt_out[0]) << ","
            << std::to_string(pt_out[1]) << "," << std::to_string(pt_out[2])
            << ")\n";

  assert(pt_out[0] == 4);
  assert(pt_out[1] == 240);
  assert(pt_out[2] == 48);
}
