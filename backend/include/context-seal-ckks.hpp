#ifndef CONTEXT_SEAL_CKKS_HPP
#define CONTEXT_SEAL_CKKS_HPP

#include <seal/seal.h>
#include <cmath>
#include <sstream>
#include <type_traits>
#include "bits.hpp"
#include "circuit.hpp"
#include "context.hpp"

namespace SHEEP {

template <typename PlaintextT>
class ContextSealCKKS : public Context<PlaintextT, seal::Ciphertext> {
 public:
  typedef PlaintextT Plaintext;
  typedef seal::Ciphertext Ciphertext;

  // constructors
  ContextSealCKKS(
      long plaintext_modulus =
          40961,  // for slots, this should be a prime congruent to 1 (mod 2N)
      long N = 8192,
      long scale_bits = 30 )
      : m_N(N), m_scale_bits(scale_bits) {
    this->m_param_name_map.insert({"N", m_N});
    this->m_param_name_map.insert({"Scale_bits", m_scale_bits});

    this->m_private_key_size = 0;
    this->m_public_key_size = 0;
    this->m_ciphertext_size = 0;

    configure();
  }

  void configure() {
    std::stringstream x;
    x << "1x^" << m_N << " + 1";
    this->m_poly_modulus = x.str();
    seal::EncryptionParameters parms(seal::scheme_type::CKKS);
    parms.set_poly_modulus_degree(m_N);
    this->m_scale = pow(2.0, m_scale_bits);

    parms.set_coeff_modulus(seal::CoeffModulus::Create(m_N, { 40, 40, 40, 40, 40 }));

    std::cout<<" about to make context"<<std::endl;
    m_context = seal::SEALContext::Create(parms);

    std::cout<<" about to generate keys"<<std::endl;
    seal::KeyGenerator keygen(m_context);
    m_public_key = keygen.public_key();
    m_secret_key = keygen.secret_key();
    m_galois_keys = keygen.galois_keys();
    m_relin_keys = keygen.relin_keys();

    std::cout<<" about to make encoder"<<std::endl;
    m_encoder = new seal::CKKSEncoder(m_context);


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
      throw std::runtime_error(
          "ContextSealCKKS::encrypt: The number of input data elements exceeds the "
          "number of slots provided by the context.");
    }

    std::vector<Plaintext> p_all(this->get_num_slots(), (Plaintext)0.);
    // We can assume p.size() < number of slots due to check above
    for (size_t i = 0; i < this->m_nslots; i++) p_all[i] = p[i % p.size()];


    seal::Plaintext pt;
    std::cout<<" p0"<<std::endl;
    m_encoder->encode(p_all, m_scale, pt);
    std::cout<<" encoded ok, got "<<this->m_nslots<<" slots"<<std::endl;
    seal::Ciphertext ct;
    m_encryptor->encrypt(pt, ct);
    this->m_ciphertext_size = sizeof(ct);
    return ct;
  }

  std::vector<Plaintext> decrypt(Ciphertext ct) {
    seal::Plaintext pt;
    m_decryptor->decrypt(ct, pt);

    std::vector<Plaintext> output(this->get_num_slots());
    m_encoder->decode(pt, output);

    std::vector<Plaintext> p(this->get_num_slots());

    for (size_t i = 0; i < this->get_num_slots(); i++) p[i] = output[i];
    return p;
  }

  std::string encrypt_and_serialize(std::vector<Plaintext> pt) {

    Ciphertext ct = encrypt(pt);
    std::stringstream ss;

    ct.save(ss);

    std::string ctstring = ss.str();

    return ctstring;

  };



  Ciphertext Add(Ciphertext a, Ciphertext b) {
    m_evaluator->add_inplace(a, b);
    return a;
  }

  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    m_evaluator->multiply_inplace(a, b);
    /// relinearize
    m_evaluator->relinearize_inplace(a, m_relin_keys);
    return a;
  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {

    m_evaluator->sub_inplace(a, b);
    return a;
  }

//  Ciphertext Negate(Ciphertext a) {
//    /// special case for bool, otherwise we get wrong answer
//    if (std::is_same<Plaintext, bool>::value)
//      return AddConstant(a,1);
//
//    m_evaluator->negate_inplace(a);
//    return a;
//  }

 // Ciphertext MultByConstant(Ciphertext a, long b) {
 //   std::vector<long> b_vec(this->get_num_slots(), b);
 //   seal::Plaintext pt;
 //   double scale = pow(2.0,30);
 //   m_encoder->encode(b_vec, scale, pt);
 //   m_evaluator->multiply_plain_inplace(a, pt);
 //   return a;
 // }
 //
 // Ciphertext AddConstant(Ciphertext a, long b) {
 //   std::vector<long> b_vec(this->get_num_slots(), b);
 //   seal::Plaintext pt;
 //   double scale = pow(2.0,30);
 //   m_encoder->encode(b_vec, scale, pt);
 //   m_evaluator->add_plain_inplace(a, pt);
 //   return a;
 // }

  //  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
  //  /// s is 0 or 1
  //  /// output is s*a + (1-s)*b
  //  Ciphertext sa = Multiply(s, a);
  //  Ciphertext one_minus_s = MultByConstant(AddConstant(s, -1L), -1L);
  //  Ciphertext one_minus_s_times_b = Multiply(one_minus_s, b);
  //  return Add(sa, one_minus_s_times_b);
  //}

  Ciphertext Rotate(Ciphertext a, long n) {
    Ciphertext b, c;
    long N = this->get_num_slots();
    if (n > 0) n = n - this->m_ninputs;
    // SEAL won't accept values for the step count outside [-N/2, N/2]
    int step_count = (-n) % (N / 2);
    m_evaluator->rotate_rows(a, step_count, m_galois_keys, b);
    return b;
  }

  // destructor
  virtual ~ContextSealCKKS() {
    /// delete everything we new-ed in the constructor
    // if (m_context != NULL) delete m_context;
    if (m_encoder != NULL) delete m_encoder;
    if (m_encryptor != NULL) delete m_encryptor;
    if (m_evaluator != NULL) delete m_evaluator;
    if (m_decryptor != NULL) delete m_decryptor;
  };

 protected:
  std::string m_poly_modulus;
  long m_N;
  long m_scale_bits;
  double m_scale;
  std::shared_ptr<seal::SEALContext> m_context;
  seal::CKKSEncoder* m_encoder;
  seal::PublicKey m_public_key;
  seal::SecretKey m_secret_key;
  seal::GaloisKeys m_galois_keys;
  seal::RelinKeys m_relin_keys;
  seal::Encryptor* m_encryptor;
  seal::Evaluator* m_evaluator;
  seal::Decryptor* m_decryptor;
};

}  // namespace SHEEP

#endif  // CONTEXT_SEAL_CKKS_HPP
