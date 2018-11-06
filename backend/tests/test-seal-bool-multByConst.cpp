#include <memory>

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "context-seal.hpp"
#include "circuit-repo.hpp"
#include "circuit-test-util.hpp"


int main(void) {

  using namespace SHEEP;
  
	ContextSeal<bool> ctx;

	/// test small postitive numbers
	std::vector<ContextSeal<bool>::Plaintext> pt_input{1};
	ContextSeal<bool>::Ciphertext ct = ctx.encrypt(pt_input);
	long const_val = 1;
	ContextSeal<bool>::Ciphertext ct_out = ctx.MultByConstant(ct, const_val);
	std::vector<ContextSeal<bool>::Plaintext> pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(1) * 1) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == 1);
	/// 0+1
	pt_input = {0};
	ct = ctx.encrypt(pt_input);
	const_val = -1;
	ct_out = ctx.MultByConstant(ct, const_val);
	pt_out = ctx.decrypt(ct_out);
	std::cout<<"D( E(0) * 1) = "<<std::to_string(pt_out[0])<<std::endl;
	assert(pt_out[0] == 0);

}
