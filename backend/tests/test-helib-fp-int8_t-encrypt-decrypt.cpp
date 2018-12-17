#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-helib.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextHElib_Fp<int8_t>::Plaintext Plaintext;
typedef ContextHElib_Fp<int8_t>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextHElib_Fp<int8_t>& context) {
  std::vector<Plaintext> pt_orig = {123,45};
  std::vector<Plaintext> pt_new = context.decrypt(context.encrypt(pt_orig));

  for (int i=0; i< pt_orig.size(); i++) {
    std::cout << std::to_string(pt_orig[i]) << " = " << std::to_string(pt_new[i])
	      << std::endl;
    assert(pt_orig[i] == pt_new[i]);
  }
}

int main(void) {
  ContextHElib_Fp<int8_t> context;
  test_single(context);
}
