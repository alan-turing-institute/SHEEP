#include <algorithm>
#include <cassert>
#include <cstdint>
#include "circuit-test-util.hpp"
#include "context-seal-ckks.hpp"
#include "simple-circuits.hpp"
#include <complex>
#include <cmath>

using namespace std::complex_literals;

using namespace SHEEP;
typedef ContextSealCKKS<std::complex<double> >::Plaintext Plaintext;
typedef ContextSealCKKS<std::complex<double> >::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextSealCKKS<std::complex<double> >& context) {
  std::complex<double> z = 1.5+1.5i;
  std::cout<<"complex number is "<<z<<std::endl;

  std::vector<Plaintext> pt_orig = {1.5 + 1.5i ,  456. + 78.1i};
  std::vector<Plaintext> pt_new = context.decrypt(context.encrypt(pt_orig));

  for (int i=0; i< pt_new.size(); i++) {
    /// std::cout << std::to_string(pt_orig[i]) << " = "
    std::cout<< pt_new[i]
	      << std::endl;
    if (i < pt_orig.size())
      assert(abs(pt_orig[i] - pt_new[i])<0.1);
  }

}

int main(void) {
  ContextSealCKKS<std::complex<double> > context;
  test_single(context);
}
