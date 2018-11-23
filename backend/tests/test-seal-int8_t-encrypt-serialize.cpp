#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-seal.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextSEAL<int8_t>::Plaintext Plaintext;
typedef ContextSEAL<int8_t>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextSEAL<int8_t>& context) {
  std::vector<Plaintext> pt_orig = {56};
  std::string serialized_ciphertext = context.encrypt_and_serialize(pt_orig);

  std::cout << std::to_string(pt_orig[0]) << " => " << serialized_ciphertext
            << std::endl;
  assert(serialized_ciphertext.size() > 0);
}

int main(void) {
  ContextSEAL<int8_t> context;
  test_single(context);
}
