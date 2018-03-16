#ifndef CONTEXT_SEAL_HPP
#define CONTEXT_SEAL_HPP

#include "circuit.hpp"
#include "context.hpp"
#include "bits.hpp"
#include <seal.h>
#include <type_traits>
#include <cmath>

namespace Sheep {
namespace Seal {

template <typename Ciphertext>
struct CiphertextWrapper { typedef Ciphertext type; };
  
template <>
struct CiphertextWrapper<bool> { typedef int type; };
  

template<typename PlaintextT>
class ContextSeal : public Context<PlaintextT, PlaintextT> {

public:
	typedef PlaintextT Plaintext;
  	typedef PlaintextT Ciphertext;
  
  // constructors
  
  ContextSeal(char* poly_modulus = "1x^2048 + 1", // This must be a power-of-2 cyclotomic polynomial described as a string, e.g. "1x^2048 + 1"
  	int security = 128,  /* This is the security level (either 128 or 192).
  							We limit ourselves to 2 predefined choices,
  							as coefficient modules are preset by SEAL for these choices.*/
	int plaintext_modulus = (1 << 8)):
	m_poly_modulus(poly_modulus),
	m_security(security),
	m_plaintext_modulus(plaintext_modulus) {

	EncryptionParameters parms;
	parms.set_poly_modulus(poly_modulus);
	if (security == 128) {
		parms.set_coeff_modulus(coeff_modulus_128(2048));
	} else if (security == 192) {
		parms.set_coeff_modulus(coeff_modulus_192(2048)); // Not sure this is correct
	} else {
	  // Unsupported context configuration
	}
	EncryptionParameters parms;
	parms.set_plain_modulus(plaintext_modulus);
	SEALContext m_context(parms);
	IntegerEncoder m_encoder(context.plain_modulus()); // We default to an IntegerEncoder with base b=2. TODO: include CRT and fractional encoder

	KeyGenerator keygen(context);
	PublicKey m_public_key = keygen.public_key();
	SecretKey m_secret_key = keygen.secret_key();
	Encryptor m_encryptor(context, public_key);
	Evaluator m_evaluator(context);
	Decryptor m_decryptor(context, secret_key);
  }

  Ciphertext encrypt(Plaintext p) {
	std::cout<<"encrypting plaintext "<<std::to_string(p)<<std::endl;
	return p; // plaintext and ciphertext are the same for this context
  }

  Plaintext decrypt(Ciphertext c) {
	return c; // plaintext and ciphertext are the same for this context
  }
  
  // In Add, Multiply, Subtract and Negate, we assume that
  // Ciphertext is either unsigned or a two's complement integer
  // type.  With a signed type, to avoid undefined behaviour,
  // cast to the corresponding unsigned type, perform the
  // operation, then cast back.  To do this with the .
  
  // Work in the corresponding unsigned type and cast
  // back, so overflow is well-defined.
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {
	if (std::is_same<Ciphertext, bool>::value) {
	  return a != b;
	} else {
	  typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	  
	  uC au = static_cast<uC>(a);
	  uC bu = static_cast<uC>(b);
	  return static_cast<Ciphertext>(au + bu);
	}
  }
  
  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
	if (std::is_same<Ciphertext, bool>::value) {
	  return a & b;
	  } else {
	  typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
		uC au = static_cast<uC>(a);
		uC bu = static_cast<uC>(b);
		return static_cast<Ciphertext>(au * bu);
	}
  }
  
  Ciphertext Subtract(Ciphertext a, Ciphertext b) {
	if (std::is_same<Ciphertext, bool>::value) {
	  return Add(a,b);
	} else {
	  typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	  
		uC au = static_cast<uC>(a);
		uC bu = static_cast<uC>(b);
		return static_cast<Ciphertext>(au - bu);
	}
  }
  
  Ciphertext Maximum(Ciphertext a, Ciphertext b) {
	return (a>=b)?a:b;
  }
  
  Ciphertext Not(Ciphertext a) {
	return !a;
  }
  
  Ciphertext Negate(Ciphertext a) {
	
	if (std::is_same<Ciphertext, bool>::value) {
	  return Not(a);
	} else {
	  typedef typename std::make_unsigned<typename CiphertextWrapper<Ciphertext>::type >::type uC;
	  uC au = static_cast<uC>(a);
	  return static_cast<Ciphertext>(-au);
	}
	
  }
  
  
  Ciphertext Compare(Ciphertext a, Ciphertext b) {
	return (a > b);
  }
  
  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
	return (s % 2)?a:b;
  }

protected:

	char* m_poly_modulus;
  	int m_security;
  	int m_plaintext_modulus;
  	SEALContext m_context;
	IntegerEncoder m_encoder;
	PublicKey m_public_key;
	SecretKey m_secret_key;
	Encryptor m_encryptor;
	Evaluator m_evaluator;
	Decryptor m_decryptor;
};

}  // Leaving Seal namespace
}  // Leaving Sheep namespace

#endif // CONTEXT_SEAL_HPP
