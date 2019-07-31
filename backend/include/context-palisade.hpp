#ifndef CONTEXT_PALISADE_HPP
#define CONTEXT_PALISADE_HPP

#include <cstdint>
#include <cmath>
#include <sstream>
#include <type_traits>
#include <palisade.h>
#include <cryptocontexthelper.h>
#include "bits.hpp"
#include "circuit.hpp"
#include "context.hpp"

namespace SHEEP {
  template <typename PlaintextT>
  class ContextPalisade :
    public Context<PlaintextT, lbcrypto::Ciphertext<lbcrypto::Poly> >
  {
  protected:
    std::string m_paramSetName;
    lbcrypto::CryptoContext<lbcrypto::Poly> m_PalisadeContext;
    lbcrypto::LPKeyPair<lbcrypto::Poly> m_keyPair;
  public:
    typedef PlaintextT Plaintext;
    typedef lbcrypto::Ciphertext<lbcrypto::Poly> Ciphertext;
    typedef std::int64_t Plaintext64;

    ContextPalisade()
    {
      uint32_t m = 22;
      // lbcrypto::PlaintextModulus p = 89; // we choose s.t. 2m|p-1 to leverage CRTArb
      lbcrypto::PlaintextModulus p = 65957;
      lbcrypto::BigInteger modulusQ("72385066601");
      lbcrypto::BigInteger modulusP(p);
      lbcrypto::BigInteger rootOfUnity("69414828251");
      lbcrypto::BigInteger bigmodulus("77302754575416994210914689");
      lbcrypto::BigInteger bigroot("76686504597021638023705542");

      auto cycloPoly = lbcrypto::GetCyclotomicPolynomial<lbcrypto::BigVector>(m, modulusQ);

      lbcrypto::ChineseRemainderTransformArb<lbcrypto::BigVector>::SetCylotomicPolynomial(cycloPoly, modulusQ);

      float stdDev = 4;

      std::shared_ptr<lbcrypto::ILParams> params(new lbcrypto::ILParams(m, modulusQ, rootOfUnity, bigmodulus, bigroot));

      lbcrypto::BigInteger bigEvalMultModulus("37778931862957161710549");
      lbcrypto::BigInteger bigEvalMultRootOfUnity("7161758688665914206613");
      lbcrypto::BigInteger bigEvalMultModulusAlt("1461501637330902918203684832716283019655932547329");
      lbcrypto::BigInteger bigEvalMultRootOfUnityAlt("570268124029534407621996591794583635795426001824");

      auto cycloPolyBig = lbcrypto::GetCyclotomicPolynomial<lbcrypto::BigVector>(m, bigEvalMultModulus);
      lbcrypto::ChineseRemainderTransformArb<lbcrypto::BigVector>::SetCylotomicPolynomial(cycloPolyBig, bigEvalMultModulus);

      uint32_t batchSize = 8;

      lbcrypto::EncodingParams encodingParams(new lbcrypto::EncodingParamsImpl(p, batchSize, lbcrypto::PackedEncoding::GetAutomorphismGenerator(m)));

      lbcrypto::PackedEncoding::SetParams(m, encodingParams);

      lbcrypto::BigInteger delta(modulusQ.DividedBy(modulusP));
      
      m_PalisadeContext =
        lbcrypto::CryptoContextFactory<lbcrypto::Poly>::genCryptoContextBFV(
          params, encodingParams, 1, stdDev, delta.ToString(), OPTIMIZED,
          bigEvalMultModulus.ToString(), bigEvalMultRootOfUnity.ToString(),
          1, 9, 1.006, bigEvalMultModulusAlt.ToString(),
          bigEvalMultRootOfUnityAlt.ToString());
      
      m_PalisadeContext->Enable(ENCRYPTION);
      m_PalisadeContext->Enable(SHE);
      m_keyPair = m_PalisadeContext->KeyGen();
      m_PalisadeContext->EvalSumKeyGen(m_keyPair.secretKey);
      m_PalisadeContext->EvalMultKeyGen(m_keyPair.secretKey);

      this->m_nslots = m/2 - 1;
    }

    void configure() { /* nothing to do */ }
    
    Ciphertext encrypt(std::vector<Plaintext> p) {
      std::vector<Plaintext64> p64(this->get_num_slots(), (Plaintext64)0);
      for (size_t i = 0; i < this->get_num_slots(); i++) {
        p64[i] = p[i % p.size()];
      }
      
      lbcrypto::Plaintext pt = m_PalisadeContext->MakePackedPlaintext(p64);
      return m_PalisadeContext->Encrypt(m_keyPair.publicKey, pt);
    }

    std::vector<Plaintext> decrypt(Ciphertext ct) {
      lbcrypto::Plaintext pt;
      m_PalisadeContext->Decrypt(m_keyPair.secretKey, ct, &pt);
      std::vector<Plaintext64> p64 = pt->GetPackedValue();
      std::vector<Plaintext> p(this->get_num_slots(), (Plaintext)0);
      if (std::is_same<Plaintext, bool>::value) {
        for (size_t i = 0; i < this->get_num_slots(); i++) {
          p[i] = (p64[i] % 2);
        }
      } else {
        for (size_t i = 0; i < this->get_num_slots(); i++) {
          p[i] = p64[i];
        }
      }
      return p;
    }

    Ciphertext Add(Ciphertext a, Ciphertext b) {
      return m_PalisadeContext->EvalAdd(a, b);
    }

    Ciphertext Multiply(Ciphertext a, Ciphertext b) {
      return m_PalisadeContext->EvalMult(a, b);
    }

    Ciphertext Subtract(Ciphertext a, Ciphertext b) {
      return m_PalisadeContext->EvalSub(a, b);
    }

    Ciphertext Negate(Ciphertext a) {
      return m_PalisadeContext->EvalNegate(a);
    }
  };
}

#endif
