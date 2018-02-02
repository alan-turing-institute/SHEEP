#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include <unordered_map>
#include <chrono>

#include "circuit.hpp"
#include "context.hpp"

// struct Wire_container {
// 	const Wire& w;
// 	Wire_container(const Wire& w_) : w(w_) {};
// };

class ContextClear : public Context<bool, bool> {
public:
	Ciphertext encrypt(Plaintext p) {
		return p; // both Plaintext and Ciphertext are typedef'd to bool
	}

	Plaintext decrypt(Ciphertext c) {
		return c;
	}
	
	Ciphertext And(Ciphertext a, Ciphertext b) {
		return a & b;
	}

	Ciphertext Or(Ciphertext a, Ciphertext b) {
		return a | b;
	}

	Ciphertext Xor(Ciphertext a, Ciphertext b) {
		return a != b;
	}
};

#endif // CONTEXT_CLEAR_HPP
