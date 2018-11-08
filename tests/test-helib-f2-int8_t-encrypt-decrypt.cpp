#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextHElib_F2<int8_t>::Plaintext Plaintext;
typedef ContextHElib_F2<int8_t>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextHElib_F2<int8_t>& context) {
  Plaintext pt_orig = 123;
  Plaintext pt_new = context.decrypt(context.encrypt(pt_orig));
  assert(pt_orig == pt_new);
}

int main(void) {
  ContextHElib_F2<int8_t> context;
  test_single(context);
}
