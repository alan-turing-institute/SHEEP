#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-seal.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"


int main(void) {

  using namespace SHEEP;
  
	ContextSeal<uint8_t> ctx;

	/// test small postitive numbers
	std::vector<ContextSeal<uint8_t>::Plaintext> pt_input{55};
	ContextSeal<uint8_t>::Ciphertext ct = ctx.encrypt(pt_input);
	long const_val = 2;
	ContextSeal<uint8_t>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);
	std::vector<ContextSeal<uint8_t>::Plaintext> pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(55) * 2) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == 110);
	/// out-of-range positive
	pt_input[0] = 120;
	ct = ctx.encrypt(pt_input);
	const_val = 121;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(120) * 121) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == 184);

	/// test slots
	pt_input = {1,60,12};
	ct = ctx.encrypt(pt_input);
	const_val = 4;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);

	std::cout << "D[E(1,60,12) * 4] == (" << std::to_string(pt_out[0]) << ","
		  << std::to_string(pt_out[1]) << "," << std::to_string(pt_out[2]) << ")\n";
	
	assert(pt_out[0] == 4);
	assert(pt_out[1] == 240);
	assert(pt_out[2] == 48);

}
