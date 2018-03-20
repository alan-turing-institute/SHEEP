#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-seal.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"


int main(void) {

  using namespace Sheep::Seal;
  
	ContextSeal<uint8_t> ctx;

	/// test small postitive numbers
	ContextSeal<uint8_t>::Plaintext pt_input = 55;
	ContextSeal<uint8_t>::Ciphertext ct = ctx.encrypt(pt_input);
	long const_val = 2;
	ContextSeal<uint8_t>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);
	ContextSeal<uint8_t>::Plaintext pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(55) * 2) = "<<std::to_string(pt_out)<<std::endl;
	assert(pt_out == 110);
	/// out-of-range positive
	pt_input = 120;
	ct = ctx.encrypt(pt_input);
	const_val = 121;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(120) * 121) = "<<std::to_string(pt_out)<<std::endl;
	assert(pt_out == 184);

	
	

}
