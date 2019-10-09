#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-seal-ckks.hpp"
#include "simple-circuits.hpp"

using namespace SHEEP;
typedef ContextSealCKKS<double>::Plaintext Plaintext;
typedef ContextSealCKKS<double>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextSealCKKS<double>& context) {
  std::vector<Plaintext> pt_orig = {123.45,456.78};
  std::vector<Plaintext> pt_new = context.decrypt(context.encrypt(pt_orig));

  for (int i=0; i< pt_new.size(); i++) {
    /// std::cout << std::to_string(pt_orig[i]) << " = "
    std::cout<< std::to_string(pt_new[i])
	      << std::endl;
    if (i < pt_orig.size())
      assert(abs(pt_orig[i] - pt_new[i])<0.1);
  }
}

int main(void) {
  ContextSealCKKS<double> context;
  test_single(context);
}
