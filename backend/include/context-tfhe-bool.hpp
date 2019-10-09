#ifndef CONTEXT_TFHE_BOOL_HPP
#define CONTEXT_TFHE_BOOL_HPP

#include <memory>
#include <complex>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "context-tfhe-common.hpp"
#include "context.hpp"

namespace SHEEP {

template <>
class ContextTFHE<bool> : public Context<bool, std::vector<CiphertextTFHE>> {
  // shared pointers, since these are handles that are referred to elsewhere
  std::shared_ptr<TFheGateBootstrappingParameterSet> parameters;
  std::shared_ptr<TFheGateBootstrappingSecretKeySet> secret_key;

  const TFheGateBootstrappingCloudKeySet *cloud_key_cptr() {
    return &secret_key.get()->cloud;
  }

 public:
  ContextTFHE(long minimum_lambda = 110)
      :  // fixed security level that works with
         // new_default_gate_bootstrapping_parameter_set, see
         // TFHE documentation and examples.
        m_minimum_lambda(minimum_lambda) {
    this->m_nslots = 1;
    this->m_param_name_map.insert({"MinimumLambda", m_minimum_lambda});
    this->m_param_name_map.insert({"NumSlots", this->m_nslots});
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
    Ciphertext ct;

    for (int i = 0; i < this->m_nslots; i++) {
      CiphertextTFHE ct_el(parameters);
      bootsSymEncrypt(ct_el, pt[i % pt.size() ], secret_key.get());
      ct.push_back(ct_el);
    }

    this->m_ciphertext_size = sizeof(*ct_el);
    return ct;
  }

  std::vector<Plaintext> decrypt(Ciphertext ct) {
    Plaintext pt_el;
    std::vector<Plaintext> pt;

    for (int i = 0; i < ct.size(); i++) {
      pt_el = bootsSymDecrypt(ct[i], secret_key.get());
      pt.push_back(pt_el);
    }

    return pt;
  }

  std::string encrypt_and_serialize(std::vector<Plaintext> pt) {

    Ciphertext ct = encrypt(pt);
    std::stringstream ss;
    const TFheGateBootstrappingParameterSet* const_params(parameters.get());
    for (int i=0; i < ct.size(); i++) {
      // CiphertextTFHE ct_el(parameters);
      export_gate_bootstrapping_ciphertext_toStream(ss, ct[i], const_params);
    }
    std::string ctstring = ss.str();

    return ctstring;

  };


  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int i = 0; i < a.size(); i++) {
      CiphertextTFHE ct_el(parameters);

      bootsAND(ct_el, a[i], b[i], cloud_key_cptr());

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Maximum(Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int i = 0; i < a.size(); i++) {
      CiphertextTFHE ct_el(parameters);

      bootsOR(ct_el, a[i], b[i], cloud_key_cptr());

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Add(Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int i = 0; i < a.size(); i++) {
      CiphertextTFHE ct_el(parameters);

      bootsXOR(ct_el, a[i], b[i], cloud_key_cptr());

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) { return Add(a, b); }

  Ciphertext Negate(Ciphertext a) {
    Ciphertext ct;

    for (int i = 0; i < a.size(); i++) {
      CiphertextTFHE ct_el(parameters);

      bootsNOT(ct_el, a[i], cloud_key_cptr());

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Select(Ciphertext s, Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if ((s.size() != a.size()) || (s.size() != b.size())) {
      throw std::runtime_error(
          "Ciphertext s, Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int i = 0; i < a.size(); i++) {
      CiphertextTFHE ct_el(parameters);

      bootsMUX(ct_el, s[i], a[i], b[i], cloud_key_cptr());

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Rotate(Ciphertext a, long n) {
    /// shift the elements of the ciphertext by n places:

    /// always rotate left, even when given positive n
    if (n > 0) n = n - this->m_ninputs;

    Ciphertext c;
    for (int i = 0; i < a.size(); i++) {
      int index = (i - n) % a.size();
      c.push_back(a[index]);
    }

    return c;
  }

 private:
  long m_minimum_lambda;
};

}  // namespace SHEEP

#endif  // CONTEXT_TFHE_BOOL_HPP
