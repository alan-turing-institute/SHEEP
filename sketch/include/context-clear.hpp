#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include "circuit.hpp"
#include "context.hpp"
#include "bits.hpp"

namespace Sheep {
namespace Clear {


template<typename PlaintextT>
class ContextClear : public Context<PlaintextT, PlaintextT> {   //plaintext and ciphertext are the same type

public:

        typedef PlaintextT Plaintext;
        typedef PlaintextT Ciphertext;
  
	Ciphertext encrypt(Plaintext p) {
	  std::cout<<"encrypting plaintext "<<std::to_string(p)<<std::endl;
	  return p; // plaintext and ciphertext are the same for this context
	}

	Plaintext decrypt(Ciphertext c) {
		return c; // plaintext and ciphertext are the same for this context
	}

	std::pair<bool,bool> HalfAdder(bool a, bool b) {
		bool sum, carry;
		sum = (a != b);
		carry = (a && b);
		return std::make_pair(sum, carry);
	}

	std::pair<bool,bool> FullAdder(bool a, bool b, bool carry_in) {
		bool sum, carry_out, s1, c1, c2;
		std::tie(s1, c1) = HalfAdder(a, b);
		std::tie(sum, c2) = HalfAdder(s1, carry_in);
		carry_out = c1 || c2;
		return std::make_pair(sum, carry_out);		
	}

	Ciphertext RippleCarryAdd(Ciphertext a, Ciphertext b) {
		Ciphertext result;
		bool sum, carry;
		std::tie(sum, carry) = HalfAdder(bit(0,a), bit(0,b));
		set_bit(0, result, sum);
		// Note that the loop starts at ONE, since we have
		// already computed the zeroth bit above
		for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
			std::tie(sum, carry) = FullAdder(bit(i,a), bit(i,b), carry);
			set_bit(i, result, sum);
		}
		return result;
	}

	Ciphertext Add(Ciphertext a, Ciphertext b) {
	  std::cout<<"Using clear context's ADD"<<std::endl;
	  return (a + b);
	}

	Ciphertext Multiply(Ciphertext a, Ciphertext b) {
	  std::cout<<"Using clear context's MULTIPLY"<<std::endl;
	  return (a * b);
	}

	Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	  std::cout<<"Using clear context's SUBTRACT"<<std::endl;
	  return (a - b);
	}

	Ciphertext Not(Ciphertext a) {
		return !a;
	}

	Ciphertext Compare(Ciphertext a, Ciphertext b) {
		return (a > b);
	}
};

}  // Leaving Clear namespace
}  // Leaving Sheep namespace
  
#endif // CONTEXT_CLEAR_HPP
