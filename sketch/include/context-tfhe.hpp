#ifndef CONTEXT_TFHE_HPP
#define CONTEXT_TFHE_HPP

#include "context.hpp"

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

namespace Sheep {
namespace TFHE {

struct CiphertextTFHE {
	std::shared_ptr<LweSample> sample;
public:
	CiphertextTFHE(std::shared_ptr<TFheGateBootstrappingParameterSet> params)
		: sample(new_gate_bootstrapping_ciphertext(params.get()),
			 [](LweSample *p){
				 delete_gate_bootstrapping_ciphertext(p);
			 })
	{ }

	// Conversion operator to allow passing of Ciphertext objects
	// to functions expecting a C pointer.
	operator LweSample*() { return sample.get(); }
};

class ContextTFHE : public Context<bool, CiphertextTFHE> {
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

	Ciphertext And(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		bootsAND(result, a, b, cloud_key_cptr());
		return result;
	}
	
	Ciphertext Or(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		bootsOR(result, a, b, cloud_key_cptr());
		return result;
	}
	
	Ciphertext Xor(Ciphertext a, Ciphertext b) {
		Ciphertext result(parameters);
		bootsXOR(result, a, b, cloud_key_cptr());
		return result;
	}

	Ciphertext Not(Ciphertext a) {
		Ciphertext result(parameters);
		bootsNOT(result, a, cloud_key_cptr());
		return result;
	}
};

}
}

#endif //CONTEXT_TFHE_HPP
