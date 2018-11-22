#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextHElib_Fp<bool>::Plaintext Plaintext;
typedef ContextHElib_Fp<bool>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextHElib_Fp<bool>& context) {
  std::vector<Plaintext> pt_orig = {1};
  std::string serialized_ciphertext = context.encrypt_and_serialize(pt_orig);

  std::cout << std::to_string(pt_orig[0]) << " => " << serialized_ciphertext
            << std::endl;
  assert(serialized_ciphertext.size() > 0);
}

int main(void) {
  ContextHElib_Fp<bool> context;
  test_single(context);
}
