#ifndef CONTEXT_HELIB_FP_HPP
#define CONTEXT_HELIB_FP_HPP

#include <unordered_map>
#include <chrono>
#include <type_traits>
#include "bits.hpp"
#include <NTL/BasicThreadPool.h>
NTL_CLIENT


#include "circuit.hpp"
#include "context.hpp"

#include "FHE.h"
#include "EncryptedArray.h"

#include "intraSlot.h"


namespace Sheep {
namespace HElib {

template <typename PlaintextT >
class ContextHElib_Fp : public Context< PlaintextT , Ctxt> {

public:

  typedef PlaintextT Plaintext;
  typedef std::vector<Plaintext> PlaintextVec; 
  typedef Ctxt Ciphertext;  


  ContextHElib_Fp(long num_levels=20, long security=80, long p=65537, long r=1, long c=3, long w=64, long d=0):
    m_L(num_levels),
    m_security(security),
    m_p(p),   // modulus of plaintext
    m_r(r),   // plaintext space 
    m_c(c),   // num columns in key-switching matrix
    m_w(w),  // Hamming weight of secret key
    m_d(d)   // unknown
  {
    /// start to build
    
    long m = FindM(m_security, m_L, m_c, m_p, m_d, 0, 0);
    
    /// initialize HElib context
    m_helib_context = new FHEcontext(m, m_p, m_r);
    /// modify context, add primes to modulus chain
    buildModChain(*m_helib_context, m_L, m_c);
    /// create secret key structure
    m_secretKey = new FHESecKey(*m_helib_context);
    
    m_publicKey = m_secretKey;  //// points to the same place
  
    ZZX G = m_helib_context->alMod.getFactorsOverZZ()[0];
    
    /// generate a secret key
    m_secretKey->GenSecKey(m_w);
    
    addSome1DMatrices(*m_secretKey);
    
    m_ea = new EncryptedArray(*m_helib_context, G);
    
    m_nslots = m_ea->size();

    m_param_name_map.insert({"security", m_security});
    m_param_name_map.insert({"L", m_L});
    m_param_name_map.insert({"p", m_p});    
    m_param_name_map.insert({"r", m_r});
    m_param_name_map.insert({"c", m_c});
    m_param_name_map.insert({"w", m_w});
    m_param_name_map.insert({"d", m_d});

   /// BITWIDTH(bool) is 8, so need to deal with this by hand...
    //// (better to specialize class?)
    if (std::is_same<Plaintext, bool>::value)
      m_bitwidth = 1;
    else
      m_bitwidth = BITWIDTH(Plaintext);

    

  }
  
  Ciphertext encrypt(PlaintextVec pt) {

    /// cast the elements of the plaintext vector into long ints for HElib
    std::vector<long> ptvec;

    //    std::cout<<" before encryption number is "<<std::to_string(ptvec[0])<<std::endl;    
    for (int i=0; i< pt.size(); i++) ptvec.push_back((long)(pt[i]));
    // std::cout<<" input ptvec has "<<std::to_string(pt.size())<<" slots "<<std::to_string(pt[0])<<std::endl;
    ////// fill up nslots with zeros//// 
    for (int i = ptvec.size(); i < this->m_nslots; i++) ptvec.push_back(0);
    
    //    std::cout<<" about to try and encrypt...  nslots is "<<std::to_string(this->m_nslots)<<" i have "<<std::to_string(pt_long.size())<<" "<<std::to_string(pt_long[0])<<std::endl;
    
    Ciphertext ct(*m_publicKey);
    m_ea->encrypt(ct, *m_publicKey, ptvec);
    return ct; 
   
  }

  PlaintextVec decrypt(Ciphertext ct) {
    
    PlaintextVec result;
 
    std::vector<long> pt_long;
    this->m_ea->decrypt(ct, *(this->m_secretKey), pt_long);
    std::cout<<" decrypting - first element is "<<std::to_string(pt_long[0])<<std::endl;
    for (int i=0; i < pt_long.size(); i++) {
      std::cout<<" decrypting "<<i<<" "<<std::to_string(pt_long[i])<<std::endl;
      long pt_transformed = pt_long[i];
      if ((pt_long[i]) > this->m_p / 2)    //// convention - treat this as a negative number
	pt_transformed = pt_long[i] - this->m_p;
      result.push_back((Plaintext)(pt_transformed  % int(pow(2,this->m_bitwidth))));
    }
    
    return result;
  }

  
  
  Ciphertext Add(Ciphertext a, Ciphertext b) {
    std::cout<<" in HElib_Fp::Add "<<std::endl;
    a += b;
    return a;

  }

  Ciphertext Subtract(Ciphertext a, Ciphertext b) {

    a -= b;
    return a;

  }
  
  
  Ciphertext Multiply(Ciphertext a, Ciphertext b) {

    a *= b;
    return a;
    
  }

  Ciphertext Negate(Ciphertext a) {

    if (this->m_bitwidth == 1)  /// special case for binary
      a.addConstant(to_ZZX(1L));
    else
      a.multByConstant(to_ZZX(-1L));  
    return a;   

  }

  Ciphertext MultByConstant(Ciphertext a, long b) {
    a.multByConstant(to_ZZX(b));
    return a;
  }

  Ciphertext AddConstant(Ciphertext a, long b) {
    a.addConstant(to_ZZX(b));
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


private:
  long m_p;     //  modulus of plaintext

  long m_d;     // this is likely to be zero

  long m_L;     // maximum number of homomorphic levels

  long m_r;     // defines plaintext space as A_{p^r} 

  long m_security;  // chosen security level for the encryption

  long m_w;     // Hamming weight of secret key

  long m_c;     // number of columns in key-switching matrix

  long m_nslots;  // number of SIMD operations that can be done at a time

  std::map<std::string, long& > m_param_name_map;
  
  EncryptedArray* m_ea; 

  FHESecKey* m_secretKey;

  const FHEPubKey* m_publicKey;

  FHEcontext* m_helib_context;

  int m_bitwidth;
  
};  /// end of class definition
  
  
}  // leaving HElib namespace
}  // leaving Sheep namespace
  
#endif // CONTEXT_HELIB_HPP
