#include <memory>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"
#include "context-helib.hpp"

int main(void) {
  using namespace SHEEP;

  ContextHElib_F2<uint8_t> ctx;

  /// test small postitive numbers
  ContextHElib_F2<uint8_t>::Plaintext pt_input = 15;
  ContextHElib_F2<uint8_t>::Ciphertext ct = ctx.encrypt(pt_input);
  long const_val = 22;
  ContextHElib_F2<uint8_t>::Ciphertext ct_out = ctx.AddConstant(ct, const_val);
  ContextHElib_F2<uint8_t>::Plaintext pt_out = ctx.decrypt(ct_out);
  std::cout << "D( E(15) + 22) = " << std::to_string(pt_out) << std::endl;
  assert(pt_out == 37);
  /// add negative numbers
  pt_input = 45;
  ct = ctx.encrypt(pt_input);
  const_val = -42;
  ct_out = ctx.AddConstant(ct, const_val);
  pt_out = ctx.decrypt(ct_out);
  std::cout << "D( E(45) - 42) = " << std::to_string(pt_out) << std::endl;
  assert(pt_out == 3);
  /// out-of-range positive
  pt_input = 220;
  ct = ctx.encrypt(pt_input);
  const_val = 121;
  ct_out = ctx.AddConstant(ct, const_val);
  pt_out = ctx.decrypt(ct_out);
  std::cout << "D( E(220) + 121) = " << std::to_string(pt_out) << std::endl;
  assert(pt_out == 85);
}
