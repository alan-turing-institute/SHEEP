#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-tfhe.hpp"
#include "simple-circuits.hpp"
#include "circuit-test-util.hpp"

using namespace SHEEP;
typedef ContextTFHE<bool>::Plaintext Plaintext;
typedef ContextTFHE<bool>::Ciphertext Ciphertext;

// Encrypt a value, decrypt the result, and check that we are
// left with the original value.
void test_single(ContextTFHE<bool>& context) {

	// Plaintext pt_false_orig = false;
	// Plaintext pt_false_new = context.decrypt(context.encrypt(pt_false_orig));
	// assert(pt_false_orig == pt_false_new);

	// Plaintext pt_true_orig = true;
	// Plaintext pt_true_new = context.decrypt(context.encrypt(pt_true_orig));
	// return (pt_true_orig == pt_true_new);


  std::vector<Plaintext> wrong = {0, 1};

  std::vector<Plaintext> pt_orig = {true, false};
  std::vector<Plaintext> pt_new = context.decrypt(context.encrypt(pt_orig));

  std::cout << "encrypt size "<< std::to_string((context.encrypt(pt_orig)).size()) << std::endl;
  // std::cout << pt_orig[1] << " = " << pt_new[1] << std::endl;

  assert(pt_orig == pt_new);

  assert(1 == 0);
};

// // Similar to test_single, but using the container interface
// bool test_containers(ContextTFHE<bool>& context) {
// 	std::vector<Plaintext> ptxt_orig{1,1,0,0,1,0,0,1,0,0,0,0,1,1,1,1};
// 	std::vector<Plaintext> ptxt_expected(ptxt_orig);

// 	std::vector<Ciphertext> ctxt;
// 	encrypt(context, ptxt_orig.begin(), ptxt_orig.end(), std::back_inserter(ctxt));

// 	std::vector<Plaintext> ptxt_actual;
// 	decrypt(context, ctxt.begin(), ctxt.end(), std::back_inserter(ptxt_actual));

// 	return all_equal(ptxt_actual, ptxt_expected);
// }

int main(void) {
	ContextTFHE<bool> context;

	test_single(context);
	//assert(test_containers(context));
}
