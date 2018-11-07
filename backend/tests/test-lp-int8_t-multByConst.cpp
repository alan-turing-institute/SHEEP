#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-lp.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"


int main(void) {

  using namespace SHEEP;

	ContextLP<int8_t> ctx;
	ctx.set_parameter("NumSlots",3);

	std::vector<ContextLP<int8_t>::Plaintext> pt_input = {55, -42, 120};
	ContextLP<int8_t>::Ciphertext ct = ctx.encrypt(pt_input);

	long const_val = 2;

	// Perform operation
	ContextLP<int8_t>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);

	// Decrypt
	std::vector<ContextLP<int8_t>::Plaintext> pt_out = ctx.decrypt(ct_out);

	assert(pt_out[0] == 110);
	assert(pt_out[1] == -84);
	assert(pt_out[2] == -16);
}
