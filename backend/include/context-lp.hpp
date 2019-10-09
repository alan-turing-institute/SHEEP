#ifndef CONTEXT_LP_HPP
#define CONTEXT_LP_HPP

#include <gmp.h>
#include "circuit.hpp"
#include "context.hpp"
extern "C" {
#include "paillier.h"
}

#include <cmath>
#include <sstream>
#include <complex>

namespace SHEEP {

template <typename PlaintextT>
class ContextLP
    : public Context<PlaintextT, std::vector<paillier_ciphertext_t> > {
 public:
  typedef PlaintextT Plaintext;
  typedef paillier_ciphertext_t CiphertextEl;
  typedef std::vector<CiphertextEl> Ciphertext;

  ContextLP(const long n = 256) : m_n(n) {
    this->m_nslots = 1;
    this->m_param_name_map.insert({"NumSlots", this->m_nslots});
    this->m_param_name_map.insert({"n", m_n});
    this->m_private_key_size = 0;
    this->m_public_key_size = 0;
    this->m_ciphertext_size = 0;
    configure();
  }

  void configure() {
    paillier_keygen(m_n, &pubKey, &secKey, paillier_get_rand_devurandom);
    this->m_public_key_size = sizeof(pubKey);
    this->m_private_key_size = sizeof(secKey);
    this->m_configured = true;
  }

  Ciphertext encrypt(std::vector<Plaintext> p) {
    Ciphertext c;
    for (int i = 0; i < this->m_nslots; i++) {
      paillier_plaintext_t* m;
      m = paillier_plaintext_from_ui(p[i % p.size()]);
      CiphertextEl* ctxt;
      ctxt = paillier_enc(NULL, pubKey, m, paillier_get_rand_devurandom);
      c.push_back(*ctxt);
    }
    return c;
  }

  std::vector<Plaintext> decrypt(Ciphertext ct) {
    std::vector<Plaintext> pt;
    for (int i = 0; i < ct.size(); i++) {
      paillier_ciphertext_t* encrypted_sum = paillier_create_enc_zero();
      paillier_plaintext_t* dec;
      dec = paillier_dec(NULL, this->pubKey, this->secKey, &ct[i]);
      /// internally libpaillier is dealing with integers, so
      /// if we want the right answer for bools, we need to take modulo 2
      if (std::is_same<Plaintext, bool>::value) {
        pt.push_back(mpz_get_ui(dec->m) % 2);
      } else {
        pt.push_back(mpz_get_ui(dec->m));
      }
    }
    return pt;
  }


  std::string encrypt_and_serialize(std::vector<Plaintext> pt) {

    Ciphertext ct = encrypt(pt);
    std::stringstream ss;
    // loop over slots
    for (int i=0; i < ct.size(); i++) {
      int ct_el_size = PAILLIER_BITS_TO_BYTES(this->pubKey->bits)*2;
      char* byte_array = (char*)paillier_ciphertext_to_bytes(ct_el_size, &ct[i]);
      ss.write(byte_array, ct_el_size);
    }
    std::string ctstring = ss.str();

    return ctstring;

  };


  Ciphertext Add(Ciphertext a, Ciphertext b) {
    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }
    Ciphertext c;
    for (int i = 0; i < a.size(); i++) {
      paillier_ciphertext_t* encrypted_sum = paillier_create_enc_zero();
      paillier_mul(this->pubKey, encrypted_sum, &a[i], &b[i]);
      c.push_back(*encrypted_sum);
    }

    return c;
  }

  Ciphertext Negate(Ciphertext a) {
    /// if we have a bool plaintext type, add enc(1)
    if (std::is_same<Plaintext, bool>::value) {
      Ciphertext enc_one;
      for (int i = 0; i < this->m_nslots; i++) {
        paillier_plaintext_t* m;
        m = paillier_plaintext_from_ui(1);
        CiphertextEl* ctxt;
        ctxt = paillier_enc(NULL, pubKey, m, paillier_get_rand_devurandom);
        enc_one.push_back(*ctxt);
      }
      return Add(a, enc_one);

    } else {
      /// otherwise just multiply by -1
      return MultByConstant(a, -1);
    }
  }

  Ciphertext MultByConstant(Ciphertext a, long b) {
    Ciphertext c;

    for (int i = 0; i < a.size(); i++) {
      paillier_ciphertext_t* product = paillier_create_enc_zero();
      paillier_plaintext_t* pt = paillier_plaintext_from_ui(b);
      paillier_exp(this->pubKey, product, &a[i], pt);
      c.push_back(*product);
    }
    return c;
  }

  Ciphertext Rotate(Ciphertext a, long n) {
    /// shift the elements of the ciphertext by n places:
    /// always rotate left - if a positive n is given (i.e. rotate right)
    /// we rotate left by ninputs - n places.
    if (n > 0) n = n - this->m_ninputs;
    Ciphertext c;
    for (int i = 0; i < a.size(); i++) {
      int index = (i - n) % a.size();
      c.push_back(a[index]);
    }
    return c;
  }

  virtual ~ContextLP(){};

 protected:
  paillier_pubkey_t* pubKey;
  paillier_prvkey_t* secKey;
  long m_n;
};

  // (dummy) specializations for double and complex<double>
template <>
class ContextLP<double> : public Context<double, std::vector<paillier_ciphertext_t>> {
 public:
  typedef double Plaintext;
  typedef std::vector<paillier_ciphertext_t> Ciphertext;

  ContextLP() {
    throw InputTypeNotSupported();
  }
  Ciphertext encrypt(std::vector<Plaintext> pt) {
    throw InputTypeNotSupported();
  }
  std::vector<Plaintext> decrypt(Ciphertext ct) {
    throw InputTypeNotSupported();
  }
};

template <>
class ContextLP<std::complex<double> >: public Context<std::complex<double>, std::vector<paillier_ciphertext_t>> {
 public:
  typedef std::complex<double> Plaintext;
  typedef std::vector<paillier_ciphertext_t> Ciphertext;

  ContextLP() {
    throw InputTypeNotSupported();
  }
  Ciphertext encrypt(std::vector<Plaintext> pt) {
    throw InputTypeNotSupported();
  }
  std::vector<Plaintext> decrypt(Ciphertext ct) {
    throw InputTypeNotSupported();
  }
};


}  // namespace SHEEP

#endif  // CONTEXT_SEAL_HPP
