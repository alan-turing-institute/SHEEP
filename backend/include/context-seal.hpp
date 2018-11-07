#ifndef CONTEXT_SEAL_HPP
#define CONTEXT_SEAL_HPP

#include "circuit.hpp"
#include "context.hpp"
#include "bits.hpp"
#include <seal/seal.h>
#include <type_traits>
#include <cmath>
#include <sstream>

namespace SHEEP {

template <typename PlaintextT>
class ContextSeal : public Context<PlaintextT, seal::Ciphertext> {

public:
	typedef PlaintextT Plaintext;
  	typedef seal::Ciphertext Ciphertext;
  typedef typename std::conditional<std::is_signed<Plaintext>::value, std::int64_t, std::uint64_t>::type Plaintext64;
  
  // constructors
  
  ContextSeal(long plaintext_modulus = 40961, // for slots, this should be a prime congruent to 1 (mod 2N)
	      long security = 128,  /* This is the security level (either 128 or 192).
				       We limit ourselves to 2 predefined choices,
				       as coefficient modules are preset by SEAL for these choices.*/
	      long N = 4096)
    :
    m_N(N),
    m_security(security),
    m_plaintext_modulus(plaintext_modulus)
  {
    this->m_param_name_map.insert({"N",m_N});    
    this->m_param_name_map.insert({"PlaintextModulus",m_plaintext_modulus});
    this->m_param_name_map.insert({"Security",m_security});
		
    this->m_private_key_size = 0;
    this->m_public_key_size = 0;
    this->m_ciphertext_size = 0;
		
    configure();
  }

  void configure() {
    std::stringstream x;
    x << "1x^" << m_N << " + 1"; 
    this->m_poly_modulus = x.str();
    seal::EncryptionParameters parms(seal::scheme_type::BFV);
    parms.set_poly_modulus_degree(m_N);
    if (m_security == 128) {
      parms.set_coeff_modulus(seal::coeff_modulus_128(m_N));
    } else if (m_security == 192) {
      parms.set_coeff_modulus(seal::coeff_modulus_192(m_N));
    } else {
      throw std::invalid_argument("Unsupported security value in ContextSeal, expected 128 or 192");
    }
	
    parms.set_plain_modulus(m_plaintext_modulus);
    m_context = seal::SEALContext::Create(parms);
    m_encoder = new seal::BatchEncoder(m_context);
	
    seal::KeyGenerator keygen(m_context);
    m_public_key = keygen.public_key();
    m_secret_key = keygen.secret_key();
    m_galois_keys = keygen.galois_keys(30);

    //// sizes of objects, in bytes
    this->m_public_key_size = sizeof(m_public_key);
    this->m_private_key_size = sizeof(m_secret_key);	
	
    m_encryptor = new seal::Encryptor(m_context, m_public_key);
    m_evaluator = new seal::Evaluator(m_context);
    m_decryptor = new seal::Decryptor(m_context, m_secret_key);

    this->m_nslots = m_encoder->slot_count();
  }

  Ciphertext encrypt(std::vector<Plaintext> p) {
    if (this->get_num_slots() < p.size()) {
      throw std::runtime_error("ContextSeal::encrypt: The number of input data elements exceeds the number of slots provided by the context.");
    }

    // the SEAL BatchEncoder can encode vectors of int64 or uint64, so
    // promote any other Plaintext type, but keep the signedness of
    // this type.  Plaintext64 is this promoted type (defined above).
    std::vector<Plaintext64> p64(this->get_num_slots(), (Plaintext64)0);
    // We can assume p.size() < number of slots due to check above
    for (size_t i = 0; i < p.size(); i++)
      p64[i] = p[i];

    seal::Plaintext pt;
    m_encoder->encode(p64, pt);
    seal::Ciphertext ct;
    m_encryptor->encrypt(pt, ct);
    this->m_ciphertext_size = sizeof(ct);
    return ct;
  }

  std::vector<Plaintext> decrypt(Ciphertext ct) {
    seal::Plaintext pt;
    m_decryptor->decrypt(ct, pt);

    std::vector<Plaintext64> p64(this->get_num_slots());
    m_encoder->decode(pt, p64);

    std::vector<Plaintext> p(this->get_num_slots());

    // TODO: rewrite to avoid explcit check for bool and
    // implementation-defined behaviour
    if (std::is_same<Plaintext, bool>::value) {
      for (size_t i = 0; i < this->get_num_slots(); i++)
	p[i] = (p64[i] % 2);
    } else {
      for (size_t i = 0; i < this->get_num_slots(); i++)
	p[i] = p64[i];
    }
    return p;
  }
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {
  	m_evaluator->add_inplace(a, b);
  	return a;
  }

  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
  	m_evaluator->multiply_inplace(a, b);
  	return a;
  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {
  	m_evaluator->sub_inplace(a, b);
  	return a;
  }
  
  Ciphertext Negate(Ciphertext a) {
  	m_evaluator->negate_inplace(a);
  	return a;
  }
  
  Ciphertext MultByConstant(Ciphertext a, long b) {
    std::vector<Plaintext64> b_vec(this->get_num_slots(), b);
    seal::Plaintext pt;
    m_encoder->encode(b_vec, pt);
    m_evaluator->multiply_plain_inplace(a, pt);
    return a;
  }

  Ciphertext AddConstant(Ciphertext a, long b) {
    std::vector<Plaintext64> b_vec(this->get_num_slots(), b);
    seal::Plaintext pt;
    m_encoder->encode(b_vec, pt);
    m_evaluator->add_plain_inplace(a, pt);
    return a;
  }

  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
    /// s is 0 or 1
    /// output is s*a + (1-s)*b
    Ciphertext sa = Multiply(s,a);
    Ciphertext one_minus_s = MultByConstant(AddConstant(s,-1L), -1L);
    Ciphertext one_minus_s_times_b = Multiply(one_minus_s, b);
    return Add(sa, one_minus_s_times_b);
  }

  Ciphertext Rotate(Ciphertext a, long n) {
    Ciphertext b, c;
    seal::Plaintext s1, s2;
    long N = this->get_num_slots();
    std::vector<Plaintext64> pre_s1(N), pre_s2(N);

    // n always even
    for (int i = 0; i < N/2; i++) {
      pre_s1[i] = ((i >= n) != (i + N/2 < n));
      pre_s1[i + N/2] = pre_s1[i];
      pre_s2[i] = 1 - pre_s1[i]; 
      pre_s2[i + N/2] = pre_s2[i];
    }

    m_encoder->encode(pre_s1, s1);
    m_encoder->encode(pre_s2, s2);

    // SEAL won't accept values for the step count outside [-N/2, N/2]
    int step_count = (-n) % (N/2);
    
    m_evaluator->rotate_rows(a, step_count, m_galois_keys, b);
    m_evaluator->rotate_columns(b, m_galois_keys, c);
    m_evaluator->multiply_plain_inplace(b, s1);
    m_evaluator->multiply_plain_inplace(c, s2);

    m_evaluator->add_inplace(b, c);
        
    return b;
  }

  // destructor
  virtual ~ContextSeal() {
    /// delete everything we new-ed in the constructor
    //if (m_context != NULL) delete m_context;
    if (m_encoder != NULL) delete m_encoder;
    if (m_encryptor != NULL) delete m_encryptor;
    if (m_evaluator != NULL) delete m_evaluator;
    if (m_decryptor != NULL) delete m_decryptor;
  };

protected:
  std::string m_poly_modulus;
  long m_security;
  long m_N;
  long m_plaintext_modulus;
  std::shared_ptr<seal::SEALContext> m_context;
  seal::BatchEncoder* m_encoder;
  seal::PublicKey m_public_key;
  seal::SecretKey m_secret_key;
  seal::GaloisKeys m_galois_keys;
  seal::Encryptor* m_encryptor;
  seal::Evaluator* m_evaluator;
  seal::Decryptor* m_decryptor;
};

}  // Leaving Sheep namespace

#endif // CONTEXT_SEAL_HPP
