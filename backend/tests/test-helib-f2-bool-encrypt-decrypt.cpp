#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextHElib_F2<bool>::Plaintext Plaintext;
typedef ContextHElib_F2<bool>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextHElib_F2<bool>& context) {
  std::vector<Plaintext> pt_orig = {1, 0, 1, 0};

  Ciphertext temp = context.encrypt(pt_orig);

  std::vector<Plaintext> pt_new = context.decrypt(temp);

  std::cout << std::to_string(pt_orig[0]) << " = " << std::to_string(pt_new[0])
            << std::endl;
  std::cout << std::to_string(pt_orig[1]) << " = " << std::to_string(pt_new[1])
            << std::endl;

  assert(pt_orig[0] == pt_new[0]);
  assert(pt_orig[1] == pt_new[1]);
  assert(pt_orig[2] == pt_new[2]);
  assert(pt_orig[3] == pt_new[3]);
}

int main(void) {
  ContextHElib_F2<bool> context;
  test_single(context);
}
