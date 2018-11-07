#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-seal.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"


int main(void) {

  using namespace SHEEP;
  
	ContextSeal<int8_t> ctx;

	/// test small postitive numbers
	std::vector<ContextSeal<int8_t>::Plaintext> pt_input{55};
	ContextSeal<int8_t>::Ciphertext ct = ctx.encrypt(pt_input);
	long const_val = 2;
	ContextSeal<int8_t>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);
	std::vector<ContextSeal<int8_t>::Plaintext> pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(55) * 2) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == 110);
	/// small negative numbers
	pt_input = {5};
	ct = ctx.encrypt(pt_input);
	const_val = -4;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(5) * -4) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == -20);
	/// out-of-range negative
	pt_input = {-125};
	ct = ctx.encrypt(pt_input);
	const_val = 21;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(-125) * 21) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == -65);
	/// out-of-range positive
	pt_input[0] = 120;
	ct = ctx.encrypt(pt_input);
	const_val = 121;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(120) * 121) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == -72);
	/// test slots
	pt_input = {1,-60,12};
	ct = ctx.encrypt(pt_input);
	const_val = -2;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);

	std::cout << "D[E(1,-60,12) * -2] == (" << std::to_string(pt_out[0]) << ","
		  << std::to_string(pt_out[1]) << "," << std::to_string(pt_out[2]) << ")\n";
	
	assert(pt_out[0] == -2);
	assert(pt_out[1] == 120);
	assert(pt_out[2] == -24);
}
