#ifndef CONTEXT_SEAL_HPP
#define CONTEXT_SEAL_HPP

#include "circuit.hpp"
#include "context.hpp"
#include "bits.hpp"
#include <seal/seal.h>
#include <type_traits>
#include <cmath>

namespace SHEEP {

template <typename PlaintextT>
class ContextSeal : public Context<PlaintextT, seal::Ciphertext> {

public:
	typedef PlaintextT Plaintext;
  	typedef seal::Ciphertext Ciphertext;
  
  // constructors
  
  ContextSeal(long plaintext_modulus = (1 << 8),
    long security = 128,  /* This is the security level (either 128 or 192).
                We limit ourselves to 2 predefined choices,
                as coefficient modules are preset by SEAL for these choices.*/
    const string poly_modulus = "1x^2048 + 1"): // This must be a power-of-2 cyclotomic polynomial described as a string, e.g. "1x^2048 + 1"):
	m_poly_modulus(poly_modulus),
	m_security(security),
	m_plaintext_modulus(plaintext_modulus) {
    this->m_param_name_map.insert({"plaintext_modulus",m_plaintext_modulus});
    this->m_param_name_map.insert({"security",m_security});

    this->m_private_key_size = 0;
    this->m_public_key_size = 0;
    this->m_ciphertext_size = 0;
    
    configure();
  }

  void configure() {
    
	seal::EncryptionParameters parms;
	parms.set_poly_modulus(m_poly_modulus);
	if (m_security == 128) {
		parms.set_coeff_modulus(seal::coeff_modulus_128(2048));
	} else if (m_security == 192) {
		parms.set_coeff_modulus(seal::coeff_modulus_192(2048)); // Not sure this is correct
	} else {
		throw std::invalid_argument("Unsupported security value in ContextSeal, expected 128 or 129");
	}
	
	parms.set_plain_modulus(m_plaintext_modulus);
	m_context = new seal::SEALContext(parms);
	m_encoder = new seal::IntegerEncoder(m_context->plain_modulus()); // We default to an IntegerEncoder with base b=2. TODO: include CRT and fractional encoder

	seal::KeyGenerator keygen(*m_context);
	m_public_key = keygen.public_key();
	m_secret_key = keygen.secret_key();

	//// sizes of objects, in bytes
	this->m_public_key_size = sizeof(m_public_key);
	this->m_private_key_size = sizeof(m_secret_key);	
	
	m_encryptor = new seal::Encryptor(*m_context, m_public_key);
	m_evaluator = new seal::Evaluator(*m_context);
	m_decryptor = new seal::Decryptor(*m_context, m_secret_key);
  }

  Ciphertext encrypt(Plaintext p) {
	seal::Plaintext pt = m_encoder->encode(p);
	seal::Ciphertext ct;
	m_encryptor->encrypt(pt, ct);
	this->m_ciphertext_size = sizeof(ct);
	return ct;
  }

  Plaintext decrypt(Ciphertext ct) {
  	seal::Plaintext pt;
    m_decryptor->decrypt(ct, pt);
    Plaintext p = m_encoder->decode_int32(pt);
    return p;
  }
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {
  	m_evaluator->add(a, b);
  	return a;
  }


  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
  	m_evaluator->multiply(a, b);
  	return a;
  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {
  	m_evaluator->sub(a, b);
  	return a;
  }
  
  Ciphertext Negate(Ciphertext a) {
  	m_evaluator->negate(a);
  	return a;
  }
  
  Ciphertext MultByConstant(Ciphertext a, long b) {
    seal::Plaintext pt = m_encoder->encode((int64_t)b);
  	m_evaluator->multiply_plain(a, pt);
    return a;
  }

  Ciphertext AddConstant(Ciphertext a, long b) {
    seal::Plaintext pt = m_encoder->encode((int64_t)b);
  	m_evaluator->multiply_plain(a, pt);
    return a;
  }

  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
    /// s is 0 or 1
    /// output is s*a + (1-s)*b
    Ciphertext sa = Multiply(s,a);
    Ciphertext one_minus_s = MultByConstant( AddConstant(s,-1L), -1L);
    Ciphertext one_minus_s_times_b = Multiply(one_minus_s, b);
    return Add(sa, one_minus_s_times_b);
  }

  // destructor
  virtual ~ContextSeal() {
    /// delete everything we new-ed in the constructor
    if (m_context != NULL) delete m_context;
    if (m_encoder != NULL) delete m_encoder;
    if (m_encryptor != NULL) delete m_encryptor;
    if (m_evaluator != NULL) delete m_evaluator;
    if (m_decryptor != NULL) delete m_decryptor;
  };

protected:
	const string m_poly_modulus;
  long m_security;
  long m_plaintext_modulus;
  seal::SEALContext* m_context;
	seal::IntegerEncoder* m_encoder;
	seal::PublicKey m_public_key;
	seal::SecretKey m_secret_key;
	seal::Encryptor* m_encryptor;
	seal::Evaluator* m_evaluator;
	seal::Decryptor* m_decryptor;
};

}  // Leaving Sheep namespace

#endif // CONTEXT_SEAL_HPP
