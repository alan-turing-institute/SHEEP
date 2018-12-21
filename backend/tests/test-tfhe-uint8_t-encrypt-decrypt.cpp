#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextTFHE<uint8_t>::Plaintext Plaintext;
typedef ContextTFHE<uint8_t>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextTFHE<uint8_t>& context) {
  context.set_parameter("NumSlots", 3);
  std::vector<Plaintext> pt_orig = {10, 0, 5};
  Ciphertext ct = context.encrypt(pt_orig);
  std::vector<Plaintext> pt_new = context.decrypt(ct);

  for (int i = 0; i < pt_orig.size(); i++) {
    std::cout << std::to_string(pt_orig[i]) << " = "
    	      << std::to_string(pt_new[i]) << std::endl;
     assert(pt_orig[i] == pt_new[i]);
  }



}

int main(void) {
  ContextTFHE<uint8_t> context;
  test_single(context);
}
