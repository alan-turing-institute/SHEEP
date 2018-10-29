#ifndef CONTEXT_TFHE_BOOL_HPP
#define CONTEXT_TFHE_BOOL_HPP

#include <memory>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "context.hpp"
#include "context-tfhe-common.hpp"

namespace SHEEP {
	
template <> class ContextTFHE<bool> : public Context<bool, std::vector<CiphertextTFHE>> {
  // const int minimum_lambda;
	// shared pointers, since these are handles that are referred to elsewhere
	std::shared_ptr<TFheGateBootstrappingParameterSet> parameters;
	std::shared_ptr<TFheGateBootstrappingSecretKeySet> secret_key;

	const TFheGateBootstrappingCloudKeySet *cloud_key_cptr() { return &secret_key.get()->cloud;
	}

public:

	ContextTFHE(long minimum_lambda=110)
		:
		// fixed security level that works with
		// new_default_gate_bootstrapping_parameter_set, see
		// TFHE documentation and examples.
		m_minimum_lambda(minimum_lambda)
	{
	  this->m_param_name_map.insert({"MinimumLambda",m_minimum_lambda});
	  this->m_private_key_size = 0;
	  this->m_public_key_size = 0;
	  this->m_ciphertext_size = 0;
	  
	  configure();
	}

	void configure() {
		  		// parameters and key, with the appropriate clean-up routines
		parameters = std::shared_ptr<TFheGateBootstrappingParameterSet>(
				   new_default_gate_bootstrapping_parameters(m_minimum_lambda),
				   [](TFheGateBootstrappingParameterSet *p) {
					   delete_gate_bootstrapping_parameters(p);
				   });
		secret_key = std::shared_ptr<TFheGateBootstrappingSecretKeySet>(
				   new_random_gate_bootstrapping_secret_keyset(parameters.get()),
				   [](TFheGateBootstrappingSecretKeySet *p) {
					     delete_gate_bootstrapping_secret_keyset(p);
				   });
		this->m_private_key_size = sizeof(*secret_key);
		this->m_public_key_size = sizeof(*secret_key);		
		this->m_configured = true;
	}
		
	Ciphertext encrypt(std::vector<Plaintext> pt) {

    CiphertextTFHE ct_el(parameters);
    Plaintext pt_el;
		Ciphertext ct;

    std::cout << "Here goes something 2!" << std::endl;

    for (int i; i < pt.size(); i ++) {
      pt_el = pt[i];

      bootsSymEncrypt(ct_el, pt_el, secret_key.get());

      ct.push_back(ct_el);
    }

    this->m_ciphertext_size = sizeof(*ct_el);	   
		return ct;
	}

	std::vector<Plaintext> decrypt(Ciphertext ct) {

    std::vector<Plaintext> c;
    
    for (int i; i < ct.size(); i ++) {
      CiphertextTFHE ct_el(parameters);
      Plaintext pt_el;


      ct_el = ct[i];

      pt_el = bootsSymDecrypt(ct_el, secret_key.get());

      std::cout << "pt_el: " << std::to_string(pt_el) << std::endl;

      c.push_back(pt_el);
    }

		return c;
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
	
	// Ciphertext Add(Ciphertext a, Ciphertext b) {
	// 	Ciphertext result(parameters);
	// 	bootsXOR(result, a, b, cloud_key_cptr());
	// 	return result;
	// }

	// Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	// 	return Add(a,b); // correct in F_2
	// }

	// Ciphertext Negate(Ciphertext a) {
	// 	Ciphertext result(parameters);
	// 	bootsNOT(result, a, cloud_key_cptr());
	// 	return result;
	// }

	// Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
	// 	//bootsMUX(LweSample* result, const LweSample* a, const LweSample* b, const LweSample* c, const TFheGateBootstrappingCloudKeySet* bk);
	// 	Ciphertext result(parameters);
	// 	bootsMUX(result, s, a, b, cloud_key_cptr());
	// 	return result;
	// }

private:
  
  long m_minimum_lambda;
  
};

}

#endif // CONTEXT_TFHE_BOOL_HPP
