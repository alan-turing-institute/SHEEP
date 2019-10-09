#ifndef CONTEXT_TFHE_INTEGER_HPP
#define CONTEXT_TFHE_INTEGER_HPP

#include <cstdint>
#include <memory>
#include <complex>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "bits.hpp"
#include "context-tfhe-common.hpp"
#include "context.hpp"

namespace SHEEP {

template <typename T>
class ContextTFHE
    : public Context<T, std::vector<CiphertextArrayTFHE<BITWIDTH(T)>>> {
  // const int minimum_lambda;
  // shared pointers, since these are handles that are referred to elsewhere
  std::shared_ptr<TFheGateBootstrappingParameterSet> parameters;
  std::shared_ptr<TFheGateBootstrappingSecretKeySet> secret_key;

  const TFheGateBootstrappingCloudKeySet *cloud_key_cptr() {
    return &secret_key.get()->cloud;
  }

 public:
  typedef Context<T, std::vector<CiphertextArrayTFHE<BITWIDTH(T)>>> Context_;

  typedef typename Context_::Plaintext Plaintext;
  typedef typename Context_::Ciphertext Ciphertext;
  typedef CiphertextArrayTFHE<BITWIDTH(T)> CiphertextEl;
  typedef CiphertextTFHE CiphertextBit;

  ContextTFHE(long minimum_lambda = 110)
      :  // fixed security level that works with
         // new_default_gate_bootstrapping_parameter_set, see
         // TFHE documentation and examples.
        m_minimum_lambda(minimum_lambda)
  // parameters and key, with the appropriate clean-up routines

  {
    this->m_nslots = 1;
    this->m_param_name_map.insert({"MinimumLambda", m_minimum_lambda});
    this->m_param_name_map.insert({"NumSlots", this->m_nslots});
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
    Ciphertext ct;
    for (int i = 0; i < this->m_nslots; i++) {
      CiphertextEl ct_el(parameters);
      for (int j = 0; j < BITWIDTH(Plaintext); j++) {
        bootsSymEncrypt(ct_el[j], bit(j, pt[i % pt.size()]), secret_key.get());
      }
      ct.push_back(ct_el);
    }
    return ct;
  }

  std::vector<Plaintext> decrypt(Ciphertext ct) {
    Plaintext pt_el;
    std::vector<Plaintext> pt;

    for (int i = 0; i < ct.size(); i++) {
      CiphertextEl ct_el(parameters);
      ct_el = ct[i];

      Plaintext pt_el(0);

      for (int j = 0; j < BITWIDTH(Plaintext); j++) {
        set_bit(j, pt_el, bootsSymDecrypt(ct_el[j], secret_key.get()));
      }

      pt.push_back(pt_el);
    }

    return pt;
  }

  std::string encrypt_and_serialize(std::vector<Plaintext> pt) {

    Ciphertext ct = encrypt(pt);
    std::stringstream ss;
    const TFheGateBootstrappingParameterSet* const_params(parameters.get());
    // loop over slots
    for (int i=0; i < ct.size(); i++) {
      CiphertextEl ct_el(parameters);
      ct_el = ct[i];
      // loop over bits
      for (int j = 0; j < BITWIDTH(Plaintext); j++) {
	export_gate_bootstrapping_ciphertext_toStream(ss, ct_el[j], const_params);
      }
    }
    std::string ctstring = ss.str();

    return ctstring;

  };

  std::pair<CiphertextBit, CiphertextBit> HalfAdder(LweSample *a,
                                                    LweSample *b) {
    CiphertextTFHE sum(parameters), carry(parameters);
    bootsXOR(sum, a, b, cloud_key_cptr());
    bootsAND(carry, a, b, cloud_key_cptr());
    return std::make_pair(sum, carry);
  }

  std::pair<CiphertextBit, CiphertextBit> FullAdder(LweSample *a, LweSample *b,
                                                    LweSample *carry_in) {
    CiphertextBit sum(parameters), carry_out(parameters), s1(parameters),
        c1(parameters), c2(parameters);

    std::tie(s1, c1) = HalfAdder(a, b);
    std::tie(sum, c2) = HalfAdder(s1, carry_in);
    bootsOR(carry_out, c1, c2, cloud_key_cptr());
    return std::make_pair(sum, carry_out);
  }

  std::pair<CiphertextBit, CiphertextBit> HalfSubtractor(LweSample *a,
                                                         LweSample *b) {
    CiphertextTFHE diff(parameters), borrow(parameters);
    bootsXOR(diff, a, b, cloud_key_cptr());
    bootsANDNY(borrow, a, b, cloud_key_cptr());
    return std::make_pair(diff, borrow);
  }

  std::pair<CiphertextBit, CiphertextBit> FullSubtractor(LweSample *x,
                                                         LweSample *y,
                                                         LweSample *borrow_in) {
    CiphertextBit diff(parameters), borrow_out(parameters), d1(parameters),
        b1(parameters), b2(parameters);

    std::tie(d1, b1) = HalfSubtractor(x, y);
    std::tie(diff, b2) = HalfSubtractor(d1, borrow_in);
    bootsOR(borrow_out, b1, b2, cloud_key_cptr());
    return std::make_pair(diff, borrow_out);
  }

  // Add implented as a ripple-carry adder
  Ciphertext Add(Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int i = 0; i < a.size(); i++) {
      CiphertextEl ct_el(parameters);
      CiphertextBit sum(parameters), carry(parameters);

      std::tie(sum, carry) = HalfAdder(a[i][0], b[i][0]);
      bootsCOPY(ct_el[0], sum, cloud_key_cptr());

      // Note that the loop starts at ONE, since we computed
      // the zeroth bit above
      for (size_t j = 1; j < BITWIDTH(Plaintext); ++j) {
        std::tie(sum, carry) = FullAdder(a[i][j], b[i][j], carry);
        bootsCOPY(ct_el[j], sum, cloud_key_cptr());
      }

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int i = 0; i < a.size(); i++) {
      CiphertextEl ct_el(parameters);
      CiphertextBit diff(parameters), borrow(parameters);

      std::tie(diff, borrow) = HalfSubtractor(a[i][0], b[i][0]);
      bootsCOPY(ct_el[0], diff, cloud_key_cptr());

      // Note that the loop starts at ONE, since we computed
      // the zeroth bit above
      for (size_t j = 1; j < BITWIDTH(Plaintext); ++j) {
        std::tie(diff, borrow) = FullSubtractor(a[i][j], b[i][j], borrow);
        bootsCOPY(ct_el[j], diff, cloud_key_cptr());
      }

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Multiply(Ciphertext a, Ciphertext b) {
    Ciphertext ct;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    for (int k = 0; k < a.size(); k++) {
      CiphertextEl ct_el(parameters);
      std::vector<CiphertextArrayTFHE<BITWIDTH(Plaintext) + 1>> carry;
      CiphertextBit sum_bit(parameters), carry_bit(parameters);

      // initialize the ciphertext array holding the carries
      for (int i = 0; i < BITWIDTH(Plaintext) + 1; i++)
        carry.emplace_back(parameters);

      for (int i = 0; i < BITWIDTH(Plaintext); i++)
        bootsCONSTANT(ct_el[i], 0, cloud_key_cptr());

      // initialize the first level of carry to zero
      for (int i = 0; i < BITWIDTH(Plaintext) + 1; i++)
        bootsCONSTANT(carry[0][i], 0, cloud_key_cptr());

      for (int i = 0; i < BITWIDTH(Plaintext); i++) {
        for (int j = 0; j <= i; j++) {
          std::tie(sum_bit, carry_bit) =
              MultiplyBit(a[k][i - j], b[k][j], ct_el[i], carry[j][i]);
          bootsCOPY(ct_el[i], sum_bit, cloud_key_cptr());
          bootsCOPY(carry[j + 1][i + 1], carry_bit, cloud_key_cptr());
        }
      }

      ct.push_back(ct_el);
    }

    return ct;
  }

  // Two's complement negation: negate each bit and add 1.
  Ciphertext Negate(Ciphertext a) {
    Ciphertext ct, const1;

    for (int i = 0; i < a.size(); i++) {
      CiphertextEl ct_el(parameters), tmp(parameters), const1_el(parameters);

      for (size_t j = 0; j < BITWIDTH(Plaintext); ++j) {
        bootsNOT(ct_el[j], a[i][j], cloud_key_cptr());
        bootsCONSTANT(const1_el[j], 0, cloud_key_cptr());
      }

      bootsCONSTANT(const1_el[0], 1, cloud_key_cptr());

      ct.push_back(ct_el);
      const1.push_back(const1_el);
    }

    return (Add(ct, const1));
  }

  // Ciphertext SubtractNaive(Ciphertext a, Ciphertext b) {
  // 	return Add(a,Negate(b));
  // }

  std::pair<CiphertextBit, CiphertextBit> MultiplyBit(LweSample *x,
                                                      LweSample *y,
                                                      LweSample *sum_in,
                                                      LweSample *carry_in) {
    CiphertextBit sum_out(parameters), carry_out(parameters),
        product_bit(parameters);
    bootsAND(product_bit, x, y, cloud_key_cptr());
    std::tie(sum_out, carry_out) = FullAdder(sum_in, product_bit, carry_in);
    return std::make_pair(sum_out, carry_out);
  }

  Ciphertext Compare(Ciphertext a, Ciphertext b) {
    Ciphertext ct, difference;

    if (a.size() != b.size()) {
      throw std::runtime_error(
          "Ciphertext a, Ciphertext b - lengths do not match.");
    }

    difference = Subtract(b, a);

    for (int i = 0; i < a.size(); i++) {
      CiphertextEl ct_el(parameters);

      // Set all result bits to zero, other than the lsb
      // (so loop counter starts at 1)
      for (size_t j = 1; j < BITWIDTH(Plaintext); j++)
        bootsCONSTANT(ct_el[j], 0, cloud_key_cptr());

      // 'a' was larger if the sign bit was set
      constexpr size_t signbit = BITWIDTH(Plaintext) - 1;
      bootsCOPY(ct_el[0], difference[i][signbit], cloud_key_cptr());

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
      CiphertextEl ct_el(parameters);

      for (int j = 0; j < BITWIDTH(Plaintext); j++) {
        bootsMUX(ct_el[j], s[i], a[i][j], b[i][j], cloud_key_cptr());
      }

      ct.push_back(ct_el);
    }

    return ct;
  }

  Ciphertext Rotate(Ciphertext a, long n) {
    /// shift the elements of the ciphertext by n places:
    /// if n is positive (i.e. rotate right), in fact we
    /// rotate left by ninputs - n places
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

// (dummy) specializations for double and complex<double>
template <>
class ContextTFHE<double> : public Context<double, std::vector<CiphertextArrayTFHE<1>>> {
 public:
  typedef double Plaintext;
  typedef std::vector<CiphertextArrayTFHE<1>> Ciphertext;

  ContextTFHE() {
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
class ContextTFHE<std::complex<double> >: public Context<std::complex<double>, std::vector<CiphertextArrayTFHE<1>>> {
 public:
  typedef std::complex<double> Plaintext;
  typedef std::vector<CiphertextArrayTFHE<1>> Ciphertext;

  ContextTFHE() {
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

#endif  // CONTEXT_TFHE_INTEGER_HPP
