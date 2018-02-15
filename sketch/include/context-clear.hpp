#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include "circuit.hpp"
#include "context.hpp"


class ContextClear : public Context<uint8_t, uint8_t> {
public:
	Ciphertext encrypt(Plaintext p) {
	  std::cout<<"encrypting plaintext "<<std::to_string(p)<<std::endl;
	  return p; // plaintext and ciphertext are the same for this context
	}

	Plaintext decrypt(Ciphertext c) {
		return c; // plaintext and ciphertext are the same for this context
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
};

#endif // CONTEXT_CLEAR_HPP
