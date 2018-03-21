#ifndef CONTEXT_TFHE_BOOL_HPP
#define CONTEXT_TFHE_BOOL_HPP

#include <memory>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "context.hpp"
#include "context-tfhe-common.hpp"

namespace SHEEP {
	
template <>
class ContextTFHE<bool> : public Context<bool, CiphertextTFHE> {
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
		bootsSymEncrypt(ct, pt, secret_key.get());
		return ct;
	}

	Plaintext decrypt(Ciphertext ct) {
		Plaintext pt = bootsSymDecrypt(ct, secret_key.get());
		return pt;
	}

	Ciphertext Multiply(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		bootsAND(result, a, b, cloud_key_cptr());
		return result;
	}
	
	Ciphertext Maximum(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		bootsOR(result, a, b, cloud_key_cptr());
		return result;
	}
	
	Ciphertext Add(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		bootsXOR(result, a, b, cloud_key_cptr());
		return result;
	}

	Ciphertext Subtract(Ciphertext a, Ciphertext b) {
		return Add(a,b); // correct in F_2
	}

	Ciphertext Negate(Ciphertext a) {
		Ciphertext result(parameters);
		bootsNOT(result, a, cloud_key_cptr());
		return result;
	}

	Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
		//bootsMUX(LweSample* result, const LweSample* a, const LweSample* b, const LweSample* c, const TFheGateBootstrappingCloudKeySet* bk);
		Ciphertext result(parameters);
		bootsMUX(result, s, a, b, cloud_key_cptr());
		return result;
	}
};

}

#endif // CONTEXT_TFHE_BOOL_HPP
