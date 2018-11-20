#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

int main(void) {
  using namespace SHEEP;

  ContextHElib_Fp<bool> ctx;

  /// test small postitive numbers
  std::vector<ContextHElib_Fp<bool>::Plaintext> pt_input = {1, 0};
  ContextHElib_Fp<bool>::Ciphertext ct = ctx.encrypt(pt_input);

  long const_val = 1;

  // Perform operation
  ContextHElib_Fp<bool>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);

  // Decrypt
  std::vector<ContextHElib_Fp<bool>::Plaintext> pt_out = ctx.decrypt(ct_out);

  assert(pt_out[0] == true);
  assert(pt_out[1] == false);

  const_val = 0;

  // Perform operation
  ct_out = ctx.MultByConstant(ct, const_val);

  // Decrypt
  pt_out = ctx.decrypt(ct_out);

  assert(pt_out[0] == false);
  assert(pt_out[1] == false);
}
