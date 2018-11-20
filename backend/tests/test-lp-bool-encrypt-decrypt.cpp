#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-lp.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextLP<bool>::Plaintext Plaintext;
typedef ContextLP<bool>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextLP<bool>& context) {
  context.set_parameter("NumSlots", 2);
  std::vector<Plaintext> pt_orig = {true, false};
  std::vector<Plaintext> pt_new = context.decrypt(context.encrypt(pt_orig));

  for (int i = 0; i < pt_new.size(); i++) {
    std::cout << pt_orig[i] << " = " << pt_new[i] << std::endl;
  }

  assert(pt_orig == pt_new);
};

int main(void) {
  ContextLP<bool> context;

  test_single(context);
}
