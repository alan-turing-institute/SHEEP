#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-helib.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

using namespace SHEEP;
typedef ContextHElib_Fp<uint8_t>::Plaintext Plaintext;
typedef ContextHElib_Fp<uint8_t>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextHElib_Fp<uint8_t>& context) {
  std::vector<Plaintext> pt_orig = {123};
  std::vector<Plaintext> pt_new = context.decrypt(context.encrypt(pt_orig));

  std::cout << std::to_string(pt_orig[0]) << " = " << std::to_string(pt_new[0]) << std::endl;
  assert(pt_orig[0] == pt_new[0]);
}

int main(void) {
  ContextHElib_Fp<uint8_t> context;
  test_single(context);
}
