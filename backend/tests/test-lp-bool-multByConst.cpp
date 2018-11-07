#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-lp.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"


int main(void) {

  using namespace SHEEP;

	ContextLP<bool> ctx;
	ctx.set_parameter("NumSlots",2);

	std::vector<ContextLP<bool>::Plaintext> pt_input = {1, 0};
	ContextLP<bool>::Ciphertext ct = ctx.encrypt(pt_input);

	long const_val = 1;

	// Perform operation
	ContextLP<bool>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);

	// Decrypt
	std::vector<ContextLP<bool>::Plaintext> pt_out = ctx.decrypt(ct_out);

	assert(pt_out[0] == 1);
	assert(pt_out[1] == 0);

}
