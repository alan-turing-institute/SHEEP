#ifndef CONTEXT_TFHE_UINT8_T_HPP
#define CONTEXT_TFHE_UINT8_T_HPP

#include <cstdint>
#include <memory>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "context.hpp"
#include "context-tfhe-common.hpp"
#include "bits.hpp"

namespace Sheep {
namespace TFHE {

template <>
class ContextTFHE<uint8_t>
	: public Context<uint8_t, CiphertextArrayTFHE<BITWIDTH(uint8_t)> >
{
	const int minimum_lambda;
	// shared pointers, since these are handles that are referred to elsewhere
	std::shared_ptr<TFheGateBootstrappingParameterSet> parameters;
	std::shared_ptr<TFheGateBootstrappingSecretKeySet> secret_key;

	const TFheGateBootstrappingCloudKeySet *cloud_key_cptr() { return &secret_key.get()->cloud; }

public:
	ContextTFHE()
		:
		// fixed security level that works with
		// new_default_gate_bootstrapping_parameter_set, see
		// TFHE documentation and examples.
		minimum_lambda(110),
		// parameters and key, with the appropriate clean-up routines
		parameters(std::shared_ptr<TFheGateBootstrappingParameterSet>(
				   new_default_gate_bootstrapping_parameters(minimum_lambda),
				   [](TFheGateBootstrappingParameterSet *p) {
					   delete_gate_bootstrapping_parameters(p);
				   })),
		secret_key(std::shared_ptr<TFheGateBootstrappingSecretKeySet>(
				   new_random_gate_bootstrapping_secret_keyset(parameters.get()),
				   [](TFheGateBootstrappingSecretKeySet *p) {
					     delete_gate_bootstrapping_secret_keyset(p);
				   }))
	{ }

	Ciphertext encrypt(Plaintext pt) {
		Ciphertext ct(parameters);
		for (int i = 0; i < BITWIDTH(Plaintext); i++) { // traits
			bootsSymEncrypt(ct[i], bit(i,pt), secret_key.get());
		}
		return ct;
	}

	Plaintext decrypt(Ciphertext ct) {
		Plaintext pt(0);
		for (int i = 0; i < BITWIDTH(Plaintext); i++) {
			set_bit(i, pt, bootsSymDecrypt(ct[i], secret_key.get()));
		}
		return pt;
	}

	// Ciphertext Multiply(Ciphertext a, Ciphertext b) {
	// 	Ciphertext result(parameters);
	// 	bootsAND(result, a, b, cloud_key_cptr());
	// 	return result;
	// }

	// Ciphertext Maximum(Ciphertext a, Ciphertext b) {
	// 	Ciphertext result(parameters);
	// 	bootsOR(result, a, b, cloud_key_cptr());
	// 	return result;
	// }

	typedef CiphertextTFHE CiphertextBit;
	
	std::pair<CiphertextBit,CiphertextBit> HalfAdder(LweSample *a, LweSample *b) {
		CiphertextTFHE sum(parameters), carry(parameters);
		bootsXOR(sum, a, b, cloud_key_cptr());
		bootsAND(carry, a, b, cloud_key_cptr());		
		return std::make_pair(sum, carry);
	}

	std::pair<CiphertextBit,CiphertextBit> FullAdder(LweSample *a, LweSample *b, LweSample *carry_in) {
		CiphertextBit sum(parameters), carry_out(parameters),
			s1(parameters), c1(parameters), c2(parameters);

		std::tie(s1, c1) = HalfAdder(a, b);
		std::tie(sum, c2) = HalfAdder(s1, carry_in);
		bootsOR(carry_out, c1, c2, cloud_key_cptr());
		return std::make_pair(sum, carry_out);
	}

	// Add implented as a ripple-carry adder
	Ciphertext Add(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		CiphertextBit sum(parameters), carry(parameters);
		std::tie(sum, carry) = HalfAdder(a[0], b[0]);
		bootsCOPY(result[0], sum, cloud_key_cptr());
		// Note that the loop starts at ONE, since we computed
		// the zeroth bit above
		for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
			std::tie(sum, carry) = FullAdder(a[i], b[i], carry);
			bootsCOPY(result[i], sum, cloud_key_cptr());
		}
		return result;
	}

	// Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	// 	return Add(a,b); // correct in F_2
	// }

	// Ciphertext Negate(Ciphertext a) {
	// 	Ciphertext result(parameters);
	// 	bootsNOT(result, a, cloud_key_cptr());
	// 	return result;
	// }
};

}
}

#endif // CONTEXT_TFHE_UINT8_T_HPP
