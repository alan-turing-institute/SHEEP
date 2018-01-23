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
	
	GateFn get_op(Gate g) {
		using namespace std::placeholders;
		switch(g) {
		case(Gate::And):
			return GateFn(std::bind(&ContextClear::And, this, _1, _2));
			break;

		case(Gate::Or):
			return GateFn(std::bind(&ContextClear::Or, this, _1, _2));
			break;

		case(Gate::Xor):
			return GateFn(std::bind(&ContextClear::Xor, this, _1, _2));
			break;

		}
		throw std::runtime_error("Unknown op");
	}

};

#endif // CONTEXT_CLEAR_HPP
