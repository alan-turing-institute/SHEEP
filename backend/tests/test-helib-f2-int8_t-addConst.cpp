#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

int main(void) {
  using namespace SHEEP;

  ContextHElib_F2<int8_t> ctx;

  std::vector<ContextHElib_F2<int8_t>::Plaintext> pt_input = {15, -42, 120};
  ContextHElib_F2<int8_t>::Ciphertext ct = ctx.encrypt(pt_input);

  long const_val = 22;

  // Perform operation
  ContextHElib_F2<int8_t>::Ciphertext ct_out = ctx.AddConstant(ct, const_val);

  // Decrypt
  std::vector<ContextHElib_F2<int8_t>::Plaintext> pt_out = ctx.decrypt(ct_out);

  assert(pt_out[0] == 37);
  assert(pt_out[1] == -20);
  assert(pt_out[2] == -114);
}
