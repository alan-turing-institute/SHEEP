#ifndef CONTEXT_TFHE_INTEGER_HPP
#define CONTEXT_TFHE_INTEGER_HPP

#include <cstdint>
#include <memory>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "context.hpp"
#include "context-tfhe-common.hpp"
#include "bits.hpp"

namespace SHEEP {

template <typename T> class ContextTFHE: public Context<T, std::vector<CiphertextArrayTFHE<BITWIDTH(T)>>> {
  // const int minimum_lambda;
	// shared pointers, since these are handles that are referred to elsewhere
	std::shared_ptr<TFheGateBootstrappingParameterSet> parameters;
	std::shared_ptr<TFheGateBootstrappingSecretKeySet> secret_key;

	const TFheGateBootstrappingCloudKeySet *cloud_key_cptr() { return &secret_key.get()->cloud; }

public:

	typedef Context<T, std::vector<CiphertextArrayTFHE<BITWIDTH(T)>>> Context_;
	
  typedef typename Context_::Plaintext Plaintext;
  typedef typename Context_::Ciphertext Ciphertext;
  
  typedef CiphertextArrayTFHE<BITWIDTH(T)> CiphertextEl;

	ContextTFHE(long minimum_lambda=110):
		// fixed security level that works with
		// new_default_gate_bootstrapping_parameter_set, see
		// TFHE documentation and examples.
		m_minimum_lambda(minimum_lambda)
		// parameters and key, with the appropriate clean-up routines

  {

		this->m_param_name_map.insert({"MinimumLambda",m_minimum_lambda});
		this->m_private_key_size = 0;
		this->m_public_key_size = 0;
		this->m_ciphertext_size = 0;

		configure();
	}

  void configure() {
    
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

		//// no public key used here? 
		this->m_public_key_size = sizeof(*secret_key);		
		this->m_configured = true;
	}

  Ciphertext encrypt(std::vector<Plaintext> pt) {
    
    CiphertextEl ct_el(parameters);
		Ciphertext ct;

    std::cout << "Here goes something!" << std::endl;

    for (int i; i < pt.size(); i ++) {
      
      std::cout << "Here goes nothing" << std::endl;

      for (int j = 0; j < BITWIDTH(Plaintext); j++) { 
			  bootsSymEncrypt(ct_el[j], bit(j, pt[i]), secret_key.get());
		  }

      ct.push_back(ct_el);
      this->m_ciphertext_size = sizeof(*ct_el);
    }

		return ct;
	}

	std::vector<Plaintext> decrypt(Ciphertext ct) {

    std::vector<Plaintext> decrypt_result;
    CiphertextEl ct_el(parameters);

    for (int i; i < ct.size(); i ++) {
      
      // ct_el = ct[i];

      

      Plaintext pt(0);

      for (int j = 0; j < BITWIDTH(Plaintext); j++) {
        set_bit(j, pt, bootsSymDecrypt(ct_el[j], secret_key.get()));
      }

      std::cout << "Here goes nothing" << std::endl;

      //decrypt_result.push_back(ct_el);
    }

		return decrypt_result;
	}


	// typedef CiphertextTFHE CiphertextBit;
	
	// std::pair<CiphertextBit,CiphertextBit> HalfAdder(LweSample *a, LweSample *b) {
	// 	CiphertextTFHE sum(parameters), carry(parameters);
	// 	bootsXOR(sum, a, b, cloud_key_cptr());
	// 	bootsAND(carry, a, b, cloud_key_cptr());		
	// 	return std::make_pair(sum, carry);
	// }

	// std::pair<CiphertextBit,CiphertextBit> HalfSubtractor(LweSample *a, LweSample *b) {
	// 	CiphertextTFHE diff(parameters), borrow(parameters);
	// 	bootsXOR(diff, a, b, cloud_key_cptr());
	// 	bootsANDNY(borrow, a, b, cloud_key_cptr());		
	// 	return std::make_pair(diff, borrow);
	// }

	// std::pair<CiphertextBit,CiphertextBit> FullAdder(LweSample *a, LweSample *b, LweSample *carry_in) {
	// 	CiphertextBit sum(parameters), carry_out(parameters),
	// 		s1(parameters), c1(parameters), c2(parameters);

	// 	std::tie(s1, c1) = HalfAdder(a, b);
	// 	std::tie(sum, c2) = HalfAdder(s1, carry_in);
	// 	bootsOR(carry_out, c1, c2, cloud_key_cptr());
	// 	return std::make_pair(sum, carry_out);
	// }

	// std::pair<CiphertextBit,CiphertextBit> FullSubtractor(LweSample *x, LweSample *y, LweSample *borrow_in) {
	// 	CiphertextBit diff(parameters), borrow_out(parameters),
	// 		d1(parameters), b1(parameters), b2(parameters);

	// 	std::tie(d1, b1) = HalfSubtractor(x, y);
	// 	std::tie(diff, b2) = HalfSubtractor(d1, borrow_in);
	// 	bootsOR(borrow_out, b1, b2, cloud_key_cptr());
	// 	return std::make_pair(diff, borrow_out);
	// }

	// // Add implented as a ripple-carry adder
	// Ciphertext Add(Ciphertext a, Ciphertext b) {
	// 	Ciphertext result(parameters);
	// 	CiphertextBit sum(parameters), carry(parameters);
	// 	std::tie(sum, carry) = HalfAdder(a[0], b[0]);
	// 	bootsCOPY(result[0], sum, cloud_key_cptr());
	// 	// Note that the loop starts at ONE, since we computed
	// 	// the zeroth bit above
	// 	for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
	// 		std::tie(sum, carry) = FullAdder(a[i], b[i], carry);
	// 		bootsCOPY(result[i], sum, cloud_key_cptr());
	// 	}
	// 	return result;
	// }

	// // Two's complement negation: negate each bit and add 1.
	// Ciphertext Negate(Ciphertext a) {
	// 	Ciphertext result(parameters), tmp(parameters), const1(parameters);
	// 	for (size_t i = 0; i < BITWIDTH(Plaintext); ++i) {
	// 		bootsNOT(result[i], a[i], cloud_key_cptr());
	// 		bootsCONSTANT(const1[i], 0, cloud_key_cptr());
	// 	}
	// 	bootsCONSTANT(const1[0], 1, cloud_key_cptr());
	// 	return Add(result, const1);
	// }

	// Ciphertext SubtractNaive(Ciphertext a, Ciphertext b) {
	// 	return Add(a,Negate(b));
	// }

	// Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	// 	Ciphertext result(parameters);
	// 	CiphertextBit diff(parameters), borrow(parameters);
	// 	std::tie(diff, borrow) = HalfSubtractor(a[0], b[0]);
	// 	bootsCOPY(result[0], diff, cloud_key_cptr());
	// 	// Note that the loop starts at ONE, since we computed
	// 	// the zeroth bit above
	// 	for (size_t i = 1; i < BITWIDTH(Plaintext); ++i) {
	// 		std::tie(diff, borrow) = FullSubtractor(a[i], b[i], borrow);
	// 		bootsCOPY(result[i], diff, cloud_key_cptr());
	// 	}
	// 	return result;
	// }

	// std::pair<CiphertextBit, CiphertextBit> MultiplyBit(
	// 	LweSample *x, LweSample *y, LweSample *sum_in, LweSample *carry_in)
	// {
	// 	CiphertextBit sum_out(parameters), carry_out(parameters),
	// 		product_bit(parameters);
	// 	bootsAND(product_bit, x, y, cloud_key_cptr());
	// 	std::tie(sum_out, carry_out) = FullAdder(sum_in, product_bit, carry_in);
	// 	return std::make_pair(sum_out, carry_out);
	// }

	// Ciphertext Multiply(Ciphertext a, Ciphertext b) {
	//  	Ciphertext result(parameters);
	// 	std::vector<CiphertextArrayTFHE<BITWIDTH(Plaintext)+1> > carry;
	// 	CiphertextBit sum_bit(parameters), carry_bit(parameters);

	// 	// initialize the ciphertext array holding the carries
	// 	for (int i = 0; i < BITWIDTH(Plaintext)+1; i++)
	// 		carry.emplace_back(parameters);

	// 	for (int i = 0; i < BITWIDTH(Plaintext); i++)
	// 		bootsCONSTANT(result[i], 0, cloud_key_cptr());

	// 	// initialize the first level of carry to zero
	// 	for (int i = 0; i < BITWIDTH(Plaintext)+1; i++)
	// 		bootsCONSTANT(carry[0][i], 0, cloud_key_cptr());

	// 	for (int i = 0; i < BITWIDTH(Plaintext); i++) {
	// 		for (int j = 0; j <= i; j++) {
	// 			std::tie(sum_bit, carry_bit) = MultiplyBit(a[i-j], b[j], result[i], carry[j][i]);
	// 			bootsCOPY(result[i], sum_bit, cloud_key_cptr());
	// 			bootsCOPY(carry[j+1][i+1], carry_bit, cloud_key_cptr());				
	// 		}
	// 	}
	//  	return result;
	// }

	// Ciphertext Compare(Ciphertext a, Ciphertext b) {
	// 	Ciphertext difference(parameters);
	// 	Ciphertext result(parameters);
	// 	// Set all result bits to zero, other than the lsb
	// 	// (so loop counter starts at 1)
	// 	for (size_t i = 1; i < BITWIDTH(Plaintext); i++)
	// 		bootsCONSTANT(result[i], 0, cloud_key_cptr());

	// 	difference = Subtract(b, a);

	// 	// 'a' was larger if the sign bit was set
	// 	constexpr size_t signbit = BITWIDTH(Plaintext) - 1;
	// 	bootsCOPY(result[0], difference[signbit], cloud_key_cptr());
	// 	return result;
	// }

	// Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
	// 	//bootsMUX(LweSample* result, const LweSample* a, const LweSample* b, const LweSample* c, const TFheGateBootstrappingCloudKeySet* bk);
	// 	Ciphertext result(parameters);
	// 	for (size_t i = 0; i < BITWIDTH(Plaintext); i++)
	// 		bootsMUX(result[i], s, a[i], b[i], cloud_key_cptr());

	// 	return result;
	// }
  private:

          long m_minimum_lambda;

};

}

#endif // CONTEXT_TFHE_INTEGER_HPP
